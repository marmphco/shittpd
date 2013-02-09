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

struct SDWorker {
    sdHandler_t handler;
    //bool working;
    int socket;
    pthread_mutex_t working;
    pthread_cond_t newrequest;

    pthread_t thread;
};

SDWorkerRef sdWorkerAlloc(sdHandler_t handler) {
    SDWorkerRef worker = malloc(sizeof(struct SDWorker));
    worker->handler = handler;
    //worker->working = false;

    pthread_mutex_init(&worker->working, NULL);
    pthread_cond_init(&worker->newrequest, NULL);

    return worker;
}

void sdWorkerDestroy(SDWorkerRef *worker) {
    if (worker != NULL && *worker != NULL) {
        free(*worker);
        *worker = NULL;
    }
}

void *work(void *arg) {
    SDWorkerRef worker = (SDWorkerRef)arg;
    pthread_mutex_lock(&worker->working);
    for(;;) {
        pthread_cond_wait(&worker->newrequest, &worker->working);
        SDLOG("recieving request");
        int sock = worker->socket;

        char buffer[4096];
        int readcount = 0;
        while ((readcount = recv(sock, buffer, 4096, 0)) > 0) {
            buffer[readcount] = '\0';
            SDLOG("%s", buffer);
            if (buffer[readcount-1] == '\n') {
                //request header is finished at \n
                break;
            }
        }
        //start response
        SDLOG("handling request");
        worker->handler(sock, buffer);
        SDLOG("closing connection");
        close(sock);
    }
    pthread_mutex_unlock(&worker->working);
    pthread_exit(0);
    return 0;
}

void sdWorkerStart(SDWorkerRef worker) {
    assert(worker != NULL);
    SDLOG("starting worker");
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&worker->thread, &attr, work, worker);

    pthread_attr_destroy(&attr);
    SDLOG("worker started");
}

void sdWorkerStop(SDWorkerRef worker) {
    assert(worker != NULL);
    SDLOG("stopping worker");
    pthread_cancel(worker->thread);
    SDLOG("worker stopped");
}

void sdWorkerSubmitRequest(SDWorkerRef worker, int socket) {
    SDLOG("submitting request to worker: %p", worker);
    pthread_mutex_lock(&worker->working);
    pthread_cond_signal(&worker->newrequest);
    pthread_mutex_unlock(&worker->working);
    worker->socket = socket;
}
