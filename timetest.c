#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv) {
    struct sockaddr_in addr_s; //init sockaddr
    char *addr; 
    short int fd=-1; //set fd to -1 for a default error (since non blocking IO)
    int port;
    fd_set fdset; //for fd settings
    struct timeval tv; //time interval struct
    int rc;  //the result of connect
    int so_error; //error from socket after select
    socklen_t len; //size of socket struct
    struct timespec tstart={0,0}, tend={0,0}; //times structs
    int seconds;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <ip> <port> <timeout_seconds>\n", argv[0]);
        return 1;
    }
    //compulsory arguments are ip, port and timeout

    addr = argv[1];
    port = atoi(argv[2]);
    seconds = atoi(argv[3]);

    addr_s.sin_family = AF_UNSPEC; //originally AF_INET, which was ipv4 only
    addr_s.sin_addr.s_addr = inet_addr(addr); //TODO: change this to use getaddrinfo
    addr_s.sin_port = htons(port);

    clock_gettime(CLOCK_MONOTONIC, &tstart); //start of time

    fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(fd, F_SETFL, O_NONBLOCK); // setup non blocking socket

    // make the connection
    rc = connect(fd, (struct sockaddr *)&addr_s, sizeof(addr_s));
    if ((rc == -1) && (errno != EINPROGRESS)) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    if (rc == 0) {
        // connection has succeeded immediately
        clock_gettime(CLOCK_MONOTONIC, &tend);
        printf("socket %s:%d connected. It took %.5f seconds\n",
            addr, port, (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec)));

        close(fd);
        return 0;
    } /*else {
        // connection attempt is in progress
    } */

    FD_ZERO(&fdset); //zero out fdset
    FD_SET(fd, &fdset); //I think this is the default arguments of fdset
    // tv.tv_sec = seconds;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;//.03 * 1000000; //microseconds, seconds * 1000000

    rc = select(fd + 1, NULL, &fdset, NULL, &tv);
    switch(rc) {
    case 1: // there is data to read
        len = sizeof(so_error);

        getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len); //retrieve socket error from fd

        if (so_error == 0) { //connected successfully
            clock_gettime(CLOCK_MONOTONIC, &tend);
            printf("socket %s:%d connected. It took %.5f seconds\n",
                addr, port, (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec)));
            close(fd);
            return 0;
        } else { // error
            fprintf(stderr, "socket %s:%d NOT connected: %s\n", addr, port, strerror(so_error));
        }
        break;
    case 0: //timeout
        fprintf(stderr, "connection timeout trying to connect to %s:%d\n", addr, port);
        break;
    }

    close(fd);
    return 0;
}

