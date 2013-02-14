/*
  sdresponse.c
*/

#include "sdresponse.h"
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "sdutil.h"
#include "sdrparse.h"

static const char *SDLOG_NAME = "Responder";

typedef enum sdctype {
    TEXT_PLAIN,
    TEXT_HTML
} sdctype_t;

typedef enum sdstatus {
    HTTP_200,
    HTTP_400,
    HTTP_403,
    HTTP_404
} sdstatus_t;

static const char *HTTP_STATUS[] = {
    "HTTP/1.0 200 OK",
    "HTTP/1.0 400 Bad Request",
    "HTTP/1.0 403 Forbidden",
    "HTTP/1.0 404 Not Found"
};

typedef struct sdresponse {
    sdstatus_t status;
    sdctype_t type;
    char path[1024];
} sdresponse_t;

void sdResponseWrite(int socket, sdresponse_t *response) {
    FILE *resource = fopen(response->path, "rb");
    fseek(resource, 0, SEEK_END);
    long size = ftell(resource);
    rewind(resource);
    char responsedata[size];
    fread(responsedata, 1, size, resource);
    fclose(resource);
    SDLOG("%s", responsedata);

    const char *responseline = HTTP_STATUS[response->status];
    const char *typeheader = "Content-Type: text/html\n";
    char lengthheader[32];
    sprintf(lengthheader, "Content-Length: %lu\n\n", size);
    write(socket, responseline, strlen(responseline));
    write(socket, typeheader, strlen(typeheader));
    write(socket, lengthheader, strlen(lengthheader));
    write(socket, responsedata, size);
            fprintf(stderr, "ok\n");

}

void sdResponseWriteError(int socket, sdresponse_t *response) {
    const char *responseline = HTTP_STATUS[response->status];
    const char *typeheader = "Content-Type: text/plain\n";
    char lengthheader[32];
    sprintf(lengthheader, "Content-Length: %lu\n\n", strlen(responseline));

    write(socket, responseline, strlen(responseline));
    write(socket, typeheader, strlen(typeheader));
    write(socket, lengthheader, strlen(lengthheader));
    write(socket, responseline, strlen(responseline));
            fprintf(stderr, "error\n");

}

void sdResponseHandleRequest(int socket, char *request) {

    //temp
    char *root = "testroot";

    //parse request
    sdrequest_t req;
    sdRequestInit(&req);
    // determine response status
    sdresponse_t res;
    if (sdRequestParse(&req, request) == 0) {
        strcpy(res.path, root);
        strcat(res.path, req.resource);
        SDLOG(" : Real Path: %s", res.path);
        if (access(res.path, F_OK) == 0) {
            if (access(res.path, R_OK) == 0) {
                SDLOG(": 200");
                //200
                res.status = HTTP_200;
            } else {
                SDLOG(": 403");
                res.status = HTTP_403;
                //403
            }
        } else {
            SDLOG(": 404");
            res.status = HTTP_404;
            //404
        }
    } else {
        SDLOG(": 400");
        res.status = HTTP_400;
        //400
    }
    sdRequestDestroy(&req);
    if (res.status == HTTP_200) {
        sdResponseWrite(socket, &res);
    } else {
        sdResponseWriteError(socket, &res);
    }
}
