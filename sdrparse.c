/*
  sdrparse.c
*/

#include "sdrparse.h"
#include "sdutil.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static const char *SDLOG_NAME = "Request Parser";

void sdRequestInit(sdrequest_t *req) {
    req->resource = NULL;
}

//everything allocated on heap for now.
int sdRequestParse(sdrequest_t *out, char *request) {

    // Request
    char *line = strtok(request, "\n");
    if (line == NULL) {
        return -1;
    }
    char method[8];
    char resource[256];
    char protocol[16];
    sscanf(line, "%s %s %s", method, resource, protocol);
    SDLOG(": Method: %s", method);
    SDLOG(": Resource: %s", resource);
    SDLOG(": Protocol: %s", protocol);
    out->resource = strdup(resource);
    line = strtok(NULL, "\n");

    // Headers
    while (line != NULL) {
        SDLOG(": %s", line);
        line = strtok(NULL, "\n");
    }
    return 0;
}

void sdRequestDestroy(sdrequest_t *request) {
    assert(request != NULL);
    if (request->resource != NULL) free(request->resource);
}
