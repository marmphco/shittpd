/*
  Oh yay.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "sdlistener.h"
#include "sddispatch.h"
#include "sdworker.h"
#include "sdutil.h"

void respond(int socket, char *request) {
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

int main(int argc, char **argv) {

    SDListenerRef listener = sdListenerAlloc(8000, 1);
    SDWorkerRef worker = sdWorkerAlloc(respond);
    sdListenerTarget(listener, worker);
    sdWorkerStart(worker);
    sdListenerStart(listener);

    getchar();
    sdListenerStop(listener);
    sdListenerDestroy(&listener);
    sdWorkerStop(worker);
    sdWorkerDestroy(&worker);

    return 0;
}
