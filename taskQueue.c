#include "taskQueue.h"

int taskEnQueue(taskQueue_t *pTaskQueue, int netFd) {
  task_t *pTask = (task_t *)calloc(1, sizeof(task_t));
  pTask->netFd = netFd;

  if (pTaskQueue->queueSize == 0) {
    pTaskQueue->pFront = pTask;
    pTaskQueue->pRear = pTask;
  } else {
    pTaskQueue->pRear->pNext = pTask;
    pTaskQueue->pRear = pTask;
  }

  ++pTaskQueue->queueSize;

  return 0;
}

int taskDeQueue(taskQueue_t *pTaskQueue) {
  task_t *pCur = pTaskQueue->pFront;
  pTaskQueue->pFront = pTaskQueue->pFront->pNext;
  free(pCur);
  --pTaskQueue->queueSize;

  return 0;
}
