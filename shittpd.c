/*
  Oh yay.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include "sdlistener.h"
#include "sddispatch.h"
#include "sdworker.h"
#include "sdutil.h"

int main(int argc, char **argv) {

   SDListenerRef listener = sdListenerAlloc(8000, 1);
   sdListenerStart(listener);
   getchar();
   sdListenerStop(listener);
   sdListenerDestroy(&listener);


   return 0;
}
