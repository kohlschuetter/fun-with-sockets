//
// accept-close-test.c
// Copyright 2024 Christian Kohlschuetter
// Apache 2.0 License
//
// Close a socket concurrently while waiting for accept(2)
//
// accept(2) behavior on
// - macOS: error ECONNABORTED is returned
// - Linux: Keeps hanging there, accept succeeds
// - IBM i (tested with PASE): Errno 3417 ("ECLOSED", undefined in PASE) is returned
// - AIX: error EINTR is returned
//
// compile with:
// cc -lpthread -o accept-close-test accept-close-test.c
//
// on IBM i (OS400 PASE), compile with:
// /QOpenSys/pkgs/bin/gcc-6 -lpthread -D_SIGSET_T  -o accept-close-test accept-close-test.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

static int sockFd;

void* thread_accept(void *vargp) {
    struct sockaddr_un remote = {0};

    socklen_t remoteSize = sizeof(remote);
    int acceptFd = accept(sockFd, (struct sockaddr*)&remote, &remoteSize);
    printf("Accepted: %i len=%i - errno=%i\n", acceptFd, remoteSize, errno);

    int errorCode;
    int errorCode_size = sizeof(errorCode);
    int ret = getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &errorCode, &errorCode_size);
    // if ret==-1 and errno=EBADF, we know that the server socket has been closed
    printf("getsockopt ret=%i errno=%i error_code=%i\n", ret, errno, errorCode);

    return NULL;
}
   
void* thread_close(void *vargp) {
    sleep(1);
    printf("Closing server socket\n");
    close(sockFd);

    return NULL;
}

int main() {
    pthread_t thread_id_accept;
    pthread_t thread_id_close;

    struct sockaddr_un addr = {
        .sun_family = AF_UNIX,
        .sun_path = "/tmp/accepttest\0"
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

    pthread_create(&thread_id_accept, NULL, thread_accept, NULL);
    pthread_create(&thread_id_close, NULL, thread_close, NULL);

    pthread_join(thread_id_accept, NULL);
    pthread_join(thread_id_close, NULL);
    exit(0);
}
