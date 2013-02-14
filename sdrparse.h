/*
  sdrparser.h
*/

#ifndef SD_REQUEST_PARSE_H
#define SD_REQUEST_PARSE_H

typedef struct sdrequest {
    //protocol
    //GET/POST
    char *resource;
    //char *referer;
    //char *host;
    //int keepalive;
} sdrequest_t;

void sdRequestInit(sdrequest_t *);
int sdRequestParse(sdrequest_t *out, char *request);
void sdRequestDestroy(sdrequest_t *);

#endif
