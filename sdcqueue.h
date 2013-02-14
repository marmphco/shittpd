/*
  sdcqueue.h
*/

#ifndef SD_CONNECTION_QUEUE_H
#define SD_CONNECTION_QUEUE_H

typedef struct SDConnectionQueue *SDConnectionQueueRef;

SDConnectionQueueRef sdConnectionQueueAlloc();
void sdConnectionQueueDestroy(SDConnectionQueueRef *);

void sdConnectionQueuePut(SDConnectionQueueRef queue, int socket);
int sdConnectionQueueWaitGet(SDConnectionQueueRef queue);
//int sdConnectionQueueGet(SDConnectionQueueRef queue);

#endif