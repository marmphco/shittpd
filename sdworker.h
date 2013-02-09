/*
  sdworker.h
*/

#ifndef SD_WORKER_H
#define SD_WORKER_H

#include "sdutil.h"

typedef struct SDWorker *SDWorkerRef;
typedef void (*sdHandler_t)(int socket, char *request);


SDWorkerRef sdWorkerAlloc(sdHandler_t);
void sdWorkerDestroy(SDWorkerRef *);

void sdWorkerStart(SDWorkerRef);
void sdWorkerStop(SDWorkerRef);

void sdWorkerSubmitRequest(SDWorkerRef, int socket);

#endif
