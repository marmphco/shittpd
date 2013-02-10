/*
  shittpd.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "sdlistener.h"
#include "sdworker.h"
#include "sdutil.h"

static int count = 0;

// this doesn;t work
void respond(int socket, char *request) {
    SDLOG("connection #%d", count++);

    //more temporary things, all in one super function
    // ahhh yis the best practices evaaarr
    // MY GOD ITS SO SLOPPY
    char resourcepath[2048] = "testroot";
    sscanf(request, "GET %s ", (char *)resourcepath+strlen(resourcepath));
    SDLOG("requested: %s", resourcepath);

    //temporary. responds with testroot/index.html
    FILE *resource = fopen(resourcepath, "rb");
    char *response;
    bool needsfree = false;
    if (resource != NULL) {
        long beginning = ftell(resource);
        fseek(resource, 0, SEEK_END);
        long size = ftell(resource)-beginning;
        fseek(resource, 0, SEEK_SET);
        response = malloc(sizeof(char)*(size+1));
        SDLOG("%lu", size);

        fread(response, 1, size, resource);
        needsfree = true;
    } else {
        //header still says 200 ok
        response = "404 not found";
    }
    SDLOG("response: %s", response);

    char *mesg = "HTTP/1.0 200 OK\n";
    char *mesg2 = "Content-Type: text/html\n";
    char mesg3[1024];
    sprintf(mesg3, "Content-Length: %lu\n\n", strlen(response));
    write(socket, mesg, strlen(mesg));
    write(socket, mesg2, strlen(mesg2));
    write(socket, mesg3, strlen(mesg3));
    write(socket, response, strlen(response));
    if (needsfree) {
        fclose(resource);
        free(response);
    }
}

int main(int argc, char **argv) {
    SDListenerRef listener = sdListenerAlloc(8000, 8);
    SDWorkerRef worker = sdWorkerAlloc(respond, sdListenerRequestQueue(listener));
    sdWorkerStart(worker);
    SDWorkerRef worker2 = sdWorkerAlloc(respond, sdListenerRequestQueue(listener));
    sdWorkerStart(worker2);
    if (!sdListenerStart(listener)) {
        return 1;
    }

    getchar();
    sdListenerStop(listener);
    sdListenerDestroy(&listener);
    sdWorkerStop(worker);
    sdWorkerDestroy(&worker);
    sdWorkerStop(worker2);
    sdWorkerDestroy(&worker2);

    return 0;
}
