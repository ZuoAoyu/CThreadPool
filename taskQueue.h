#ifndef __TASK_QUEUE
#define __TASK_QUEUE

#include "head.h"

typedef struct task_s {
  int netFd;
  struct task_s *pNext;
} task_t;

typedef struct taskQueue_s {
  task_t *pFront;
  task_t *pRear;
  int queueSize;         // 当前任务个数
  pthread_mutex_t mutex; // 锁
  pthread_cond_t cond;
} taskQueue_t;

int taskEnQueue(taskQueue_t *pTaskQueue, int netFd);
int taskDeQueue(taskQueue_t *pTaskQueue);

#endif
