/*
  listener.c
*/

#include "sdlistener.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <pthread.h>

#include "sdutil.h"

/*
  Logging
*/
static const char *SDLOG_NAME = "Listener";

struct SDListener {
    struct sockaddr_in address;
    int port;
    int socket;
    int backlog;

    pthread_t thread;
    SDConnectionQueueRef queue;
};

SDListenerRef sdListenerAlloc(int port) {
    SDListenerRef listener = malloc(sizeof(struct SDListener));

    listener->address.sin_family = AF_INET;
    listener->address.sin_addr.s_addr = INADDR_ANY;
    listener->address.sin_port = htons(port);

    listener->socket = socket(AF_INET, SOCK_STREAM, 0);

    if (listener->socket == -1) {
        SDLOG(" %p: Error creating socket", listener);
        return NULL;
    }

    listener->backlog = SOMAXCONN;
    listener->port = port;
    listener->queue = NULL;

    return listener;
}

void sdListenerDestroy(SDListenerRef *listener) {
    assert(listener != NULL);
    assert(*listener != NULL);

    free(*listener);
    *listener = NULL;
}

void *sdListen(void *arg) {
    SDListenerRef listener = (SDListenerRef)arg;
    for (;;) {
        struct sockaddr_in caddr;
        socklen_t caddrsize = sizeof(caddr);
        SDLOG(" %p: Listening for connections", listener);

        int sock = accept(listener->socket, (struct sockaddr *)&caddr, &caddrsize);
        if (sock == -1) {
            SDLOG(" %p: Error accepting connection", listener);
            break;
        }

        SDLOG(" %p: socket %d: Accepted connection", listener, sock);
        sdConnectionQueuePut(listener->queue, sock);
        SDLOG(" %p: socket %d: Enqueued connection", listener, sock);
    }
    pthread_exit(0);
    return 0;
}

int sdListenerStart(SDListenerRef listener, SDConnectionQueueRef queue) {
    assert(listener != NULL);
    assert(queue != NULL);

    int status = bind(listener->socket, 
                      (struct sockaddr *)&listener->address,
                      sizeof(listener->address));
    if (status == -1) {
        SDLOG(" %p: Error binding socket, port: %d", listener, listener->port);
        return 0;
    }

    status = listen(listener->socket, listener->backlog);
    if (status == -1) {
        SDLOG(" %p: Error listening on socket, port: %d", listener, listener->port);
        return 0;
    }
    SDLOG(" %p: Started", listener);

    listener->queue = queue;
    pthread_attr_t newthreadattr;
    pthread_attr_init(&newthreadattr);
    pthread_attr_setdetachstate(&newthreadattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&listener->thread, &newthreadattr, sdListen, listener);
    pthread_attr_destroy(&newthreadattr);

    return 1;
}

void sdListenerStop(SDListenerRef listener) {
    assert(listener != NULL);

    pthread_cancel(listener->thread);
    close(listener->socket); //implicitly cancels the listener thread
    SDLOG(" %p: Stopped", listener);
}

        //int flags = fcntl(listener->socket, F_GETFL, 0);
        //fcntl(listener->socket, F_SETFL, flags | O_NONBLOCK);

        //hacky and temporary
        /*int kq = kqueue();
        struct kevent changelist;
        EV_SET(&changelist, listener->socket, EVFILT_READ, EV_ADD, 1000, 0, NULL);
        struct kevent event;
        struct timespec ts = {1,0};
        kevent(kq, &changelist, 1, &event, 1, NULL);*/
        //hacky and temporary
