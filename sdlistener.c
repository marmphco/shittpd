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
};

void defaultResponse(int socket) {
    char *ret = "SHITTPD 0.01 The Super Hyper Interwebs Text Transfer Protocol Daemon";
    char *mesg = "HTTP/1.0 200 OK\n";
    char *mesg2 = "Content-Type: text/plain\n";
    char mesg3[1024];
    sprintf(mesg3, "Content-Length: %lu\n\n", strlen(ret));
    write(socket, mesg, strlen(mesg));
    write(socket, mesg2, strlen(mesg2));
    write(socket, mesg3, strlen(mesg3));
    write(socket, ret, strlen(ret));
}

void *handleRequest(void *csock_ptr) {
    int csock = *(int *)(csock_ptr);
    char buffer[4096];
    int readcount = 0;
    while ((readcount = recv(csock, buffer, 4096, 0)) > 0) {
        buffer[readcount] = '\0';
        SDLOG("%s", buffer);
        if (buffer[readcount-1] == '\n') {
            //request header is finished at \n
            break;
        }
    }
    //start response
    defaultResponse(csock);
    SDLOG("closing connection");
    close(csock);
    pthread_exit(0);
    return 0;
}

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

    SDLOG("listener allocated");
    return listener;
}

void sdListenerDestroy(SDListenerRef *listener) {
    if (*listener != NULL) {
        free(*listener);
        *listener = NULL;
    }
    SDLOG("listener destroyed");
}

void *sdListen(void *arg) {
    SDListenerRef listener = (SDListenerRef)arg;
    for (;;) {
        struct sockaddr_in caddr;
        socklen_t caddrsize = sizeof(caddr);
        int csock = accept(listener->socket, (struct sockaddr *)&caddr, &caddrsize);
        if (csock == -1) {
            if (listener->stopped == 1) {
                SDLOG("Listener stopped");
            } else {
                SDLOG("Error accepting connection");
            }
            break;
        }

        SDLOG("accepted connection");

        pthread_attr_t newthreadattr;
        pthread_attr_init(&newthreadattr);
        pthread_attr_setdetachstate(&newthreadattr, PTHREAD_CREATE_DETACHED);

        pthread_t newthread;
        pthread_create(&newthread, &newthreadattr, handleRequest, &csock);

        pthread_attr_destroy(&newthreadattr);
    }
    pthread_exit(0);
    return 0;
}

void sdListenerStart(SDListenerRef listener) {
    int status = bind(listener->socket, 
                      (struct sockaddr *)&listener->address,
                      sizeof(listener->address));
    if (status == -1) {
        SDLOG("Error binding socket on port: %d", listener->port);
        return;
    }

    status = listen(listener->socket, listener->backlog);
    if (status == -1) {
        SDLOG("Error listening on socket on port: %d", listener->port);
        return;
    }
    SDLOG("listener starting");
    listener->stopped = 0;
    pthread_attr_t newthreadattr;
    pthread_attr_init(&newthreadattr);
    pthread_attr_setdetachstate(&newthreadattr, PTHREAD_CREATE_DETACHED);

    pthread_t listenThread;
    pthread_create(&listenThread, &newthreadattr, sdListen, listener);

    pthread_attr_destroy(&newthreadattr);
}

void sdListenerStop(SDListenerRef listener) {
    SDLOG("listener stopping");
    listener->stopped = 1; //pthread_cancel?
    close(listener->socket); //implicitly cancels the listener thread
}
