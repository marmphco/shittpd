/*
  llistener.h
*/

#ifndef SD_LISTENER_H
#define SD_LISTENER_H

#include "sdutil.h"
#include "sdrequestqueue.h"

typedef struct SDListener *SDListenerRef;

SDListenerRef sdListenerAlloc(int port, int backlog);
void sdListenerDestroy(SDListenerRef *);

//binds the socket and starts listening
bool sdListenerStart(SDListenerRef);
void sdListenerStop(SDListenerRef);

SDRequestQueueRef sdListenerRequestQueue(SDListenerRef);

#endif
