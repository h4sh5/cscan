#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "extraUtils.h"
#include "color.h"
#include "port.h"

/*
thing to do:
- connect to server and print out result (failed/succeed)
- proper argument parsing

*/

pthread_mutex_t mutex;
float timeout;
int threadMax = 15; //max threads is 15 by default

enum ExitCode {
    USAGE = 1,
    BAD_TIMEOUT = 2,
    BAD_THREADS = 3,
};

struct ScanInput {
    char *host;
    char port[6];
};


/* thread-safe function to connect scan a single host and a port 
params:
    struct ScanInput - the host and port to scan
    TODO: make sockets time out

*/
void *scan(void *arg) {

    struct ScanInput *info = (struct ScanInput *) arg;
    char port[6];
    char host[50];

    snprintf(port, sizeof(port), "%s", info->port);
    snprintf(host, sizeof(host), "%s", info->host);
    free(info);
#ifdef DEBUG
    fprintf(stderr, "scanning %s:%s\n", host, port);
#endif
    pthread_mutex_unlock(&mutex);
    int result = try_connect(host, port, timeout);

#ifdef DEBUG
    fprintf(stderr, "[scan] try_connect returned: %i\n", result);
#endif
    if (result < 0) {
        if (result == SOCK_CREATE) {
            fprintf(stderr, "failed creating socket\n");
        } else if (result == CONN) { //in this case port closed
            // fprintf(stderr, "%sport %s probably closed.%s\n", IRED, port, RESET);
        } else if (result == ADDRINFO) {
            fprintf(stderr, "getaddrinfo failed.\n");
        } else if (result == TIMEOUT) { //in this case port is likely filtered, unless timeout is too low
            // fprintf(stderr, "%s\n", );
        }
        
        return NULL;
    }
    printf("%s%s:%s is open!%s\n", IGREEN, host, port, RESET);
    return NULL;
}

/* scan all the ports from 0 - 65535 */
void scan_all_ports(char *host) {
    // fprintf(stderr, "scanning all ports!\n");
    pthread_mutex_init(&mutex, NULL);

    pthread_t threads[65536];
    int tcount = 0;

    for (int i = 1; i < 65536; i+=threadMax) { //do maths properly
        for (int j = i; j < i + threadMax; j++) {
            if (j < 65536) {
                struct ScanInput *info = malloc(sizeof(struct ScanInput));
                info->host = host;
                snprintf(info->port, sizeof(info->port), "%i", j);
                //lock mutex upon start
                pthread_mutex_lock(&mutex);
                if (pthread_create(&threads[j], NULL, scan, (void *)info) < 0) {
                    fprintf(stderr, "%scould not create thread for port: %s%s\n", 
                            RED, info->port, RESET);
                }
                tcount++;
            }
        }

        for (int j = i; j < i + threadMax; j++) {
            if (j < 65536) { //overflow guard
                pthread_join(threads[j], NULL);
            }
            // pthread_detach(threads[j]); //this is likely evil
        }
    }
}

//scan a list of ports
void scan_port_list(char *host, const int *ports, int count) {
    pthread_t threads[count];
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < count; i++) {
        struct ScanInput *info = malloc(sizeof(struct ScanInput));
        info->host = host;
        snprintf(info->port, sizeof(info->port), "%i", ports[i]);
        pthread_mutex_lock(&mutex);
        if (pthread_create(&threads[i], NULL, scan, (void *)info) < 0) {
                fprintf(stderr, "%scould not create thread for port: %s%s\n", 
                        RED, info->port, RESET);
        }
    }

    for (int i = 0; i < count; i++) {
        pthread_join(threads[i], NULL);
    }

}

int main(int argc, char **argv) {

    if (argc < 4) {
        fprintf(stderr, "usage: %s <host> <ports> <timeout> [threads]\n", argv[0]);
        exit(USAGE);
    }

    char *end;

    if (argc == 5) {
        int threads = strtol(argv[4], &end, 10);
        if (strcmp(argv[4], end) == 0) {
            fprintf(stderr, "Bad threads\n");
            exit(BAD_THREADS);
        }
        threadMax = threads;
    }

    char *host = argv[1];
    char *port = argv[2];
    //parsing timeout and putting it into a global var
    
    timeout = strtof(argv[3], &end);
    if (strcmp(argv[3], end) == 0) { //timeout convert to float failed
        fprintf(stderr, "Bad timeout: %s\n", argv[3]);
        exit(BAD_TIMEOUT);
    }

    if (strcmp(port, "all") == 0) {
        scan_all_ports(host);
    } else if (strcmp(port, "top") == 0) {
        int count = sizeof(top) / sizeof(int);
        scan_port_list(host, &top[0], count);
    } else {
        struct ScanInput *info = malloc(sizeof(struct ScanInput));
        info->host = host;
        snprintf(info->port, sizeof(info->port), "%s", port);
        scan((void *) info);
    }

   
}
