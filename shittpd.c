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

static int count = 0;
static const char *SDLOG_NAME = "shittpd";

// this doesn;t work well
void respond(int socket, char *request) {
    SDLOG("connection #%d", count++);

    sdrequest_t req;
    sdRequestInit(&req);
    if (sdRequestParse(&req, request) == -1) {
        char *response = "400 Bad Request";
        char *mesg = "HTTP/1.0 400 Bad Request\n";
        char *mesg2 = "Content-Type: text/plain\n";
        char mesg3[32];
        sprintf(mesg3, "Content-Length: %lu\n\n", strlen(response));
        write(socket, mesg, strlen(mesg));
        write(socket, mesg2, strlen(mesg2));
        write(socket, mesg3, strlen(mesg3));
        write(socket, response, strlen(response));
    } else {
        if (access(req.resource, R_OK) == 0) {
            struct stat statstr;
            stat(req.resource, &statstr);

            FILE *resource = fopen(req.resource, "rb");
            fseek(resource, 0, SEEK_END);
            long size = ftell(resource);
            rewind(resource);
            char response[size];
            fread(response, 1, size, resource);
            fclose(resource);
            SDLOG("%s", response);

            char *mesg = "HTTP/1.0 200 OK\n";
            char *mesg2 = "Content-Type: text/html\n";
            char mesg3[32];
            sprintf(mesg3, "Content-Length: %lu\n\n", size);
            write(socket, mesg, strlen(mesg));
            write(socket, mesg2, strlen(mesg2));
            write(socket, mesg3, strlen(mesg3));
            write(socket, response, size);
        } else {
            char *response = "404 Not Found";
            char *mesg = "HTTP/1.0 404 Not Found\n";
            char *mesg2 = "Content-Type: text/plain\n";
            char mesg3[32];
            sprintf(mesg3, "Content-Length: %lu\n\n", strlen(response));
            write(socket, mesg, strlen(mesg));
            write(socket, mesg2, strlen(mesg2));
            write(socket, mesg3, strlen(mesg3));
            write(socket, response, strlen(response));
        }
    }
    sdRequestDestroy(&req);
    close(socket);
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
