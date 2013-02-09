/*
  util.h
*/

#ifndef SD_UTIL_H
#define SD_UTIL_H

#include <stdio.h>

#define SD_DEBUG

#ifdef SD_DEBUG
#define SDLOG(fmt, ...) fprintf(stderr, fmt"\n", ##__VA_ARGS__)
#else
#define SDLOG(fmt) 
#endif

#endif