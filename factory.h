#ifndef __FACTORY
#define __FACTORY
#include "taskQueue.h"

// 线程间共享的数据
typedef struct factory_s {
  pthread_t *tidArr;
  int threadNum;
  taskQueue_t taskQueue;
  int runningFlag;
} factory_t;

// 通常把构建实际对象的函数称为 工厂函数
int factoryInit(factory_t *pFactory, int threadNum);

#endif
