/*
  llistener.h
*/

#ifndef SD_LISTENER_H
#define SD_LISTENER_H

typedef struct SDListener *SDListenerRef;
typedef void (*sdHandler_t)(int socket);

SDListenerRef sdListenerAlloc(int port, int backlog);
void sdListenerDestroy(SDListenerRef *);

//binds the socket and starts listening
void sdListenerStart(SDListenerRef);
void sdListenerStop(SDListenerRef);

#endif
