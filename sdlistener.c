/*
  listener.c
*/

#include "sdlistener.h"
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <pthread.h>

#include "sdutil.h"

struct SDListener {
    struct sockaddr_in address;
    int port;
    int socket;
    int backlog;
    int stopped;

    //temporary
    SDWorkerRef target;
};

SDListenerRef sdListenerAlloc(int port, int backlog) {
    SDListenerRef listener = malloc(sizeof(struct SDListener));

    listener->address.sin_family = AF_INET;
    listener->address.sin_addr.s_addr = INADDR_ANY;
    listener->address.sin_port = htons(port);

    listener->socket = socket(AF_INET, SOCK_STREAM, 0);
    if (listener->socket == -1) {
        SDLOG("Error creating socket");
        return NULL;
    }

    listener->backlog = backlog;
    listener->port = port;
    listener->stopped = 0;
    listener->target = NULL;

    SDLOG("listener allocated");
    return listener;
}

void sdListenerDestroy(SDListenerRef *listener) {
    if (*listener != NULL) {
        free(*listener);
        *listener = NULL;
    }
    SDLOG("Listener %p: Destroyed", *listener);
}

void *sdListen(void *arg) {
    SDListenerRef listener = (SDListenerRef)arg;
    for (;;) {
        struct sockaddr_in caddr;
        socklen_t caddrsize = sizeof(caddr);
        SDLOG("Listener %p: Listening for connections", listener);
        int sock = accept(listener->socket, (struct sockaddr *)&caddr, &caddrsize);
        if (sock == -1) {
            if (listener->stopped == 1) {
                SDLOG("Listener stopped");
            } else {
                SDLOG("Error accepting connection");
            }
            break;
        }

        SDLOG("Listener %p: socket %d: Accepted connection", listener, sock);
        sdWorkerSubmitRequest(listener->target, sock);
        struct timespec t = {0, 1};
        nanosleep(&t, NULL);
    }
    pthread_exit(0);
    return 0;
}

bool sdListenerStart(SDListenerRef listener) {
    int status = bind(listener->socket, 
                      (struct sockaddr *)&listener->address,
                      sizeof(listener->address));
    if (status == -1) {
        SDLOG("Error binding socket on port: %d", listener->port);
        return false;
    }

    status = listen(listener->socket, listener->backlog);
    if (status == -1) {
        SDLOG("Error listening on socket on port: %d", listener->port);
        return false;
    }
    SDLOG("Listener %p: Starting", listener);
    listener->stopped = 0;
    pthread_attr_t newthreadattr;
    pthread_attr_init(&newthreadattr);
    pthread_attr_setdetachstate(&newthreadattr, PTHREAD_CREATE_DETACHED);

    pthread_t listenThread;
    pthread_create(&listenThread, &newthreadattr, sdListen, listener);

    pthread_attr_destroy(&newthreadattr);

    return true;
}

void sdListenerStop(SDListenerRef listener) {
    SDLOG("listener %p: Stopping", listener);
    listener->stopped = 1; //pthread_cancel?
    close(listener->socket); //implicitly cancels the listener thread
}

void sdListenerTarget(SDListenerRef listener, SDWorkerRef worker) {
    listener->target = worker;
}
