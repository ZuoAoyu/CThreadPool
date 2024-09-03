#ifndef __WORKER
#define __WORKER

#include "factory.h"
#include "head.h"

void makeWorker(factory_t *pFactory);
void *threadFunc(void *pArgs);

int handleEvent(int netFd);

#endif
