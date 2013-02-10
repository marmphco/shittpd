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

static const int REQUEST_MAX_SIZE = 4096;

struct SDWorker {
    sdHandler_t handler;

    int socket;
    pthread_mutex_t busy_mutex;
    pthread_cond_t newrequest_cond;

    pthread_t thread;
};

SDWorkerRef sdWorkerAlloc(sdHandler_t handler) {
    SDWorkerRef worker = malloc(sizeof(struct SDWorker));
    worker->handler = handler;

    pthread_mutex_init(&worker->busy_mutex, NULL);
    pthread_cond_init(&worker->newrequest_cond, NULL);

    return worker;
}

void sdWorkerDestroy(SDWorkerRef *worker) {
    assert(worker != NULL && *worker != NULL);
    pthread_mutex_destroy(&(*worker)->busy_mutex);
    pthread_cond_destroy(&(*worker)->newrequest_cond);
    free(*worker);
    *worker = NULL;
}

void *work(void *arg) {
    SDWorkerRef worker = (SDWorkerRef)arg;
    pthread_mutex_lock(&worker->busy_mutex);
    for(;;) {
        SDLOG("Worker %p: Waiting", worker);
        pthread_cond_wait(&worker->newrequest_cond, &worker->busy_mutex);
        SDLOG("Worker %p: socket %d : Recieving request", worker, worker->socket);
        int sock = worker->socket;

        char buffer[REQUEST_MAX_SIZE];
        int readcount = 0;
        readcount = recv(sock, buffer, REQUEST_MAX_SIZE, 0);
        buffer[readcount] = '\0';
        SDLOG("%s", buffer);

        //start response
        SDLOG("Worker %p: socket %d : Handling request", worker, worker->socket);
        worker->handler(sock, buffer);
        SDLOG("Worker %p: socket %d : Closing connection", worker, worker->socket);
        close(sock);
    }
    pthread_mutex_unlock(&worker->busy_mutex);
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

void sdWorkerSubmitRequest(SDWorkerRef worker, int socket) {
    SDLOG("Worker %p: socket %d: Submitting connection", worker, socket);
    pthread_mutex_lock(&worker->busy_mutex);
    worker->socket = socket;
    pthread_cond_signal(&worker->newrequest_cond);
    pthread_mutex_unlock(&worker->busy_mutex);
    SDLOG("Worker %p: socket %d: Submitted connection", worker, socket);
}
