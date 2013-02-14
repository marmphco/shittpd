/*
  llistener.h
*/

#ifndef SD_LISTENER_H
#define SD_LISTENER_H

#include "sdutil.h"
#include "sdcqueue.h"

typedef struct SDListener *SDListenerRef;

SDListenerRef sdListenerAlloc(int port);
void sdListenerDestroy(SDListenerRef *);

//binds the socket and starts listening
int sdListenerStart(SDListenerRef, SDConnectionQueueRef);
void sdListenerStop(SDListenerRef);

#endif
