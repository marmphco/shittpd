/*
  sddispatch.h
*/

#ifndef SD_DISPATCH_H
#define SD_DISPATCH_H

typedef struct SDDispatch *SDDispatchRef;

SDDispatchRef sdDispatchAlloc();
void sdDispatchDestroy(SDDispatchRef *);

#endif
