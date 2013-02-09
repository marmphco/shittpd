/*
  llistener.h
*/

#ifndef SD_LISTENER_H
#define SD_LISTENER_H

#include "sdworker.h"

typedef struct SDListener *SDListenerRef;

SDListenerRef sdListenerAlloc(int port, int backlog);
void sdListenerDestroy(SDListenerRef *);

//binds the socket and starts listening
bool sdListenerStart(SDListenerRef);
void sdListenerStop(SDListenerRef);

void sdListenerTarget(SDListenerRef, SDWorkerRef);

#endif
