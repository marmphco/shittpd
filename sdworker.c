/*
  sdworker.c
  will be reworked with a request queue. 
  For now, only a single request at a time.
*/

#include "sdworker.h"
#include "sdresponse.h"

#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

static const int REQUEST_MAX_SIZE = 4096;
static const char *SDLOG_NAME = "Worker";

struct SDWorker {
    sdHandler_t handler;
    SDConnectionQueueRef queue;
    pthread_t thread;
};

SDWorkerRef sdWorkerAlloc(sdHandler_t handler) {
    SDWorkerRef worker = malloc(sizeof(struct SDWorker));
    worker->handler = handler;
    return worker;
}

void sdWorkerDestroy(SDWorkerRef *worker) {
    assert(worker != NULL);
    assert(*worker != NULL);
    free(*worker);
    *worker = NULL;
}

void *work(void *arg) {
    SDWorkerRef worker = (SDWorkerRef)arg;
    for(;;) {
        SDLOG(" %p: Waiting", worker);
        int sock = sdConnectionQueueWaitGet(worker->queue);
        if (sock == -1) {
            SDLOG(" %p: Request already taken", worker);
            continue;
        }
        SDLOG(" %p: socket %d : Recieving request", worker, sock);

        char buffer[REQUEST_MAX_SIZE];
        int readcount = 0;
        readcount = recv(sock, buffer, REQUEST_MAX_SIZE, 0);
        buffer[readcount] = '\0';

        SDLOG(" %p: socket %d : Handling request", worker, sock);
        worker->handler(sock, buffer);
        SDLOG(" %p: socket %d : Closing connection", worker, sock);

        close(sock);
    }
    pthread_exit(0);
    return 0;
}

void sdWorkerStart(SDWorkerRef worker, SDConnectionQueueRef queue) {
    assert(worker != NULL);
    assert(queue != NULL);

    worker->queue = queue;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&worker->thread, &attr, work, worker);
    pthread_attr_destroy(&attr);
    SDLOG(" %p: Started", worker);
}

void sdWorkerStop(SDWorkerRef worker) {
    assert(worker != NULL);
    pthread_cancel(worker->thread);
    SDLOG(" %p: Stopped", worker);
}
