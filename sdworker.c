/*
  sdworker.c
  will be reworked with a request queue. 
  For now, only a single request at a time.
*/

#include "sdworker.h"

#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>

static const int REQUEST_MAX_SIZE = 4096;

struct SDWorker {
    sdHandler_t handler;
    SDRequestQueueRef queue;
    pthread_t thread;
};

SDWorkerRef sdWorkerAlloc(sdHandler_t handler, SDRequestQueueRef queue) {
    SDWorkerRef worker = malloc(sizeof(struct SDWorker));
    worker->handler = handler;

    worker->queue = queue;

    return worker;
}

void sdWorkerDestroy(SDWorkerRef *worker) {
    assert(worker != NULL && *worker != NULL);
    free(*worker);
    *worker = NULL;
}

void *work(void *arg) {
    SDWorkerRef worker = (SDWorkerRef)arg;
    for(;;) {
        SDLOG("Worker %p: Waiting", worker);
        int sock = sdRequestQueueGet(worker->queue);
        if (sock == -1) {
            SDLOG("Worker %p: Request already taken", worker);
            continue;
        }
        SDLOG("Worker %p: socket %d : Recieving request", worker, sock);

        //hacky and temporary
       /* int kq = kqueue();
        struct kevent changelist;
        EV_SET(&changelist, sock, EVFILT_READ, EV_ADD, 1000, 0, NULL);
        struct kevent event;
        struct timespec ts = {1,0};
        kevent(kq, &changelist, 1, &event, 1, NULL);*/
        //hacky and temporary

        char buffer[REQUEST_MAX_SIZE];
        int readcount = 0;
        readcount = recv(sock, buffer, REQUEST_MAX_SIZE, 0);
        buffer[readcount] = '\0';
        SDLOG("###\n%s\n###", buffer);

        //start response
        //hacky and temporary
        //EV_SET(&changelist, sock, EVFILT_WRITE, EV_ADD, 1000, 0, NULL);
        //kevent(kq, &changelist, 1, &event, 1, NULL);
        //hacky and temporary
        SDLOG("Worker %p: socket %d : Handling request", worker, sock);
        worker->handler(sock, buffer);
        SDLOG("Worker %p: socket %d : Closing connection", worker, sock);
        //sleep(1);
        close(sock);
    }
    pthread_exit(0);
    return 0;
}

void sdWorkerStart(SDWorkerRef worker) {
    assert(worker != NULL);
    SDLOG("Worker %p: Starting", worker);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&worker->thread, &attr, work, worker);

    pthread_attr_destroy(&attr);
    SDLOG("Worker %p: Started", worker);
}

void sdWorkerStop(SDWorkerRef worker) {
    assert(worker != NULL);
    SDLOG("Worker %p: Stopping", worker);
    pthread_cancel(worker->thread);
    SDLOG("Worker %p: Stopped", worker);
}
