//
// accept-timeout-test.c
// Copyright 2024 Christian Kohlschuetter
// Apache 2.0 License
//
// Call accept(2) with a timeout and see what happens
// (Times out on Linux; hangs on BSD, also hangs on FreeBSD Linuxulator)
//
// compile with:
// cc -o accept-timeout-test accept-timeout-test.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <errno.h>

static int sockFd;

int main() {
    pthread_t thread_id_accept;
    pthread_t thread_id_close;

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = "/tmp/accepttimeouttest\0"
    };

    unlink(addr.sun_path);

    sockFd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockFd == -1) {
        perror("socket");
        return errno;
    }
    if(bind(sockFd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) != 0) {
        perror("bind");
        return errno;
    }
    if(listen(sockFd, 10) != 0) {
        perror("listen");
        return errno;
    }

    struct timeval timeout = {
        .tv_sec = 1,
        .tv_usec = 0
    };

    if(setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
        perror("setsockopt SO_RCVTIMEO");
        return errno;
    }

    int acceptSock;
    if((acceptSock = accept(sockFd, NULL, 0)) == -1) {
        perror("accept");
        return errno;
    }

    printf("Accept result: %i\n", acceptSock);

    exit(0);
}
