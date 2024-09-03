#include "worker.h"
#include "transFile.h"

void makeWorker(factory_t *pFactory) {
  for (int i = 0; i < pFactory->threadNum; ++i) {
    pthread_create(pFactory->tidArr + i, NULL, threadFunc, (void *)pFactory);
    printf("new worker %d tid = %lu\n", i, *(pFactory->tidArr + i));
  }
}

void *threadFunc(void *pArgs) {
  factory_t *pFactory = (factory_t *)pArgs;

  while (1) {
    pthread_mutex_lock(&pFactory->taskQueue.mutex);

    while (pFactory->taskQueue.queueSize == 0) {
      pthread_cond_wait(&pFactory->taskQueue.cond, &pFactory->taskQueue.mutex);
    }

    printf("Get Task!\n");

    int netFd = pFactory->taskQueue.pFront->netFd;
    taskDeQueue(&pFactory->taskQueue);

    pthread_mutex_unlock(&pFactory->taskQueue.mutex);

    handleEvent(netFd);
    printf("pthread done! tid = %lu\n", pthread_self());
  }
}

int handleEvent(int netFd) {
  recvFile(netFd);
  close(netFd);

  return 0;
}
