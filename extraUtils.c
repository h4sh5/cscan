#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#ifdef __MACH__
    #define TCP_USER_TIMEOUT TCP_CONNECTIONTIMEOUT
#endif
#include "extraUtils.h"

/* test if string starts with the string start 
params:
    string - the string to test
    start - the string to test if string starts with
*/
bool starts_with(char* string, char* start) {
    size_t startLen = strlen(start);
    size_t stringLen = strlen(string);
    if (startLen <= stringLen && strncmp(start, string, startLen) == 0) {
        return true;
    }
    return false;
}


/* create a sock fd based on the given host and port.
params:
    host
    port

returns:
    sock (a positive integer): if the creation of the sock is successful
    error code if error.
*/ 
int sock_create(char* host, char* port) {
    struct addrinfo hints, *res0;
    int error;
    int sock;
#ifdef DEBUG
    fprintf(stderr, "%s:%s\n", host, port);
#endif
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(host, port, &hints, &res0);

    if (error != 0) {
        return -1;
    }
    sock = -1;
    if ((sock = socket(res0->ai_family, res0->ai_socktype, 
            res0->ai_protocol)) == -1) {
        return -1;
    }

    freeaddrinfo(res0);

    return sock;
}

/* attempt to connect to a server at the given host and port. 
params:
    host
    port
    timeout (in seconds, a float)

returns:
    -1: if there was an error
    sock: the fd of the socket

modularize possibilities:
- put socket creation in another function 
*/
int try_connect(char *host, char *port,
        float timeout) {

    struct addrinfo hints, *res0;
    int error;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    error = getaddrinfo(host, port, &hints, &res0);

    if (error != 0) {
        return ADDRINFO;
    }
    
    int sock = -1;
    sock = socket(res0->ai_family, res0->ai_socktype, res0->ai_protocol);
    if (sock == -1) {
        freeaddrinfo(res0);
        perror("sock: ");
        return SOCK_CREATE;
    }

    // struct timeval timeout;      
    // timeout.tv_sec = 1; 
    // timeout.tv_usec = 0;

    if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {// setup non blocking IO
        perror("fcntl: ");
        return CONN;
        return FCNTL;
    } 

#ifdef DEBUG
    fprintf(stderr, "[try_connect] socket fd: %i\n", sock);
#endif

    // make the connection
    int rc = connect(sock, (struct sockaddr*)res0->ai_addr, 
            res0->ai_addrlen);

    if ((rc == -1) && (errno != EINPROGRESS)) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        freeaddrinfo(res0);
        close(sock);
        return CONN;
    }

    if (rc == 0) { //immediate success
        close(sock);
        return sock;
    }

    fd_set fdset; //for fd settings

    FD_ZERO(&fdset); //zero out fdset
    FD_SET(sock, &fdset); //I think this is the default arguments of fdset, as at this point it is zero
    socklen_t len; //size of socket struct

    struct timeval tv; //time interval struct 
    int so_error; //error from socket after select

#ifdef DEBUG
      fprintf(stderr, "[try_connect] timeout: %i\n", (int) timeout);
#endif

    //you can only use tv_sec or tv_usec, not both.
    if (timeout >= 1) {
        tv.tv_sec = (long int) timeout;
        tv.tv_usec = 0;
#ifdef DEBUG
      fprintf(stderr, "[try_connect] setting int timeout: %li\n", tv.tv_sec);
#endif
        
    } else {
        tv.tv_sec = 0;
        tv.tv_usec = timeout * 1000000; //usec is microseconds
    }
    
    rc = select(sock + 1, NULL, &fdset, NULL, &tv);
    switch(rc) {
        case 1: // there is data to read
            len = sizeof(so_error);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len); //retrieve socket error from sock fd

            if (so_error == 0) { //connected successfully
                break;
            } else { // error
                close(sock);
                return CONN;
            }
            break;
        case 0: //timeout
            close(sock);
            return TIMEOUT;
    }
    close(sock);
    freeaddrinfo(res0);
    return sock;

}





