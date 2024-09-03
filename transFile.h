#ifndef __TRANS_FILE
#define __TRANS_FILE

#include "head.h"

typedef struct train_s {
  int dataLength;
  char buf[1000];
} train_t;

int recvn(int netFd, void *buf, int total);

int recvFile(int netFd);
int sendFile(int netFd);

#endif
