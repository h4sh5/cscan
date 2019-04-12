#ifndef EXTRA_H
#define EXTRA_H

#include <stdbool.h>
#include <unistd.h>

enum ErrorCode {
    SOCK_CREATE = -1,
    CONN = -2,
    TIMEOUT = -3,
    ADDRINFO = -4,
    FCNTL = -5,
};


bool starts_with(char *string, char *start);
int sock_create(char *host, char *port);
int try_connect(char *host, char *port, float timeout);

#endif
