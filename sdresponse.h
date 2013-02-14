/*
  sdresponse.h
*/

#ifndef SD_RESPONSE_H
#define SD_RESPONSE_H

typedef struct sdroptions {
    char root[64];
} sdroptions_t;

void sdResponseHandleRequest(int socket, char *request);

#endif
