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
    TEXT_HTML,
    TEXT_CSS,
    IMAGE_PNG
} sdctype_t;

static const char *CONTENT_TYPE[] = {
    "text/plain",
    "text/html",
    "text/css",
    "image/png",
};

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

sdctype_t sdResponseContentType(char *ext) {
    //crappy linear search through mime types
    //cause I don't want to make a map right now.
    sdctype_t type;
    if(strcmp(ext, "png") == 0) {
        type = IMAGE_PNG;
    } else if(strcmp(ext, "html") == 0) {
        type = TEXT_HTML;
    } else if(strcmp(ext, "css") == 0) {
        type = TEXT_CSS;
    } else {
        type = TEXT_PLAIN;
    }
    return type;
}

void sdResponseWrite(int socket, sdresponse_t *response) {
    FILE *resource = fopen(response->path, "rb");
    fseek(resource, 0, SEEK_END);
    long size = ftell(resource);
    rewind(resource);
    char responsedata[size];
    fread(responsedata, 1, size, resource);
    fclose(resource);
    SDLOG("%s", responsedata);

    char responseline[32];
    sprintf(responseline, "%s\n", HTTP_STATUS[response->status]);
    char typeheader[32];
    sprintf(typeheader, "Content-Type: %s\n", CONTENT_TYPE[response->type]);
    char lengthheader[32];
    sprintf(lengthheader, "Content-Length: %lu\n\n", size);
    write(socket, responseline, strlen(responseline));
    write(socket, typeheader, strlen(typeheader));
    write(socket, lengthheader, strlen(lengthheader));
    write(socket, responsedata, size);
}

void sdResponseWriteError(int socket, sdresponse_t *response) {
    char responseline[32];
    sprintf(responseline, "%s\n", HTTP_STATUS[response->status]);
    char typeheader[32];
    sprintf(typeheader, "Content-Type: %s\n", CONTENT_TYPE[TEXT_PLAIN]);
    char lengthheader[32];
    sprintf(lengthheader, "Content-Length: %lu\n\n", strlen(responseline));

    write(socket, responseline, strlen(responseline));
    write(socket, typeheader, strlen(typeheader));
    write(socket, lengthheader, strlen(lengthheader));
    write(socket, responseline, strlen(responseline));
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
            struct stat statbuf;
            stat(res.path, &statbuf);
            if (S_ISDIR(statbuf.st_mode)) {
                strcat(res.path, "/index.html");
            }
            if (access(res.path, R_OK) == 0) {
                SDLOG(": 200");
                //200
                res.status = HTTP_200;
                char *ext = strrchr(res.path, '.')+1;
                res.type = sdResponseContentType(ext);

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
