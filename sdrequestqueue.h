/*
  sdrequestqueue.h
  Maybe it's counter productive just passing a bunch of 
  simple file descriptors around like this?
*/

#ifndef SD_REQUEST_QUEUE_H
#define SD_REQUEST_QUEUE_H

typedef struct SDRequestQueue *SDRequestQueueRef;

SDRequestQueueRef sdRequestQueueAlloc();
void sdRequestQueueDestroy(SDRequestQueueRef *);

void sdRequestQueuePut(SDRequestQueueRef queue, int socket);
int sdRequestQueueGet(SDRequestQueueRef queue);

#endif