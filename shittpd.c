/*
  shittpd.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "sdlistener.h"
#include "sdworker.h"
#include "sdutil.h"
#include "sdrparse.h"
#include "sdresponse.h"

static int count = 0;
static const char *SDLOG_NAME = "shittpd";

// this doesn't work well
void respond(int socket, char *request) {
    SDLOG("connection #%d", count++);
    sdResponseHandleRequest(socket, request);
}

int main(int argc, char **argv) {

    if (argc != 3) {
        fprintf(stderr, "usage: shittpd port workers\n");
        return 1;
    }

    int port = atoi(argv[1]);
    int worker_count = atoi(argv[2]);

    SDListenerRef listener = sdListenerAlloc(port);
    SDConnectionQueueRef queue = sdConnectionQueueAlloc();

    SDWorkerRef workers[worker_count];
    for (int i = 0; i < worker_count; ++i) {
        workers[i] = sdWorkerAlloc(respond);
        sdWorkerStart(workers[i], queue);
    }
    if (!sdListenerStart(listener, queue)) {
        return 1;
    }

    getchar();
    sdListenerStop(listener);
    sdListenerDestroy(&listener);
    sdConnectionQueueDestroy(&queue);
    for (int i = 0; i < worker_count; ++i) {
        sdWorkerStop(workers[i]);
        sdWorkerDestroy(&workers[i]);
    }
    return 0;
}
