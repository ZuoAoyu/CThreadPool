#include "worker.h"
#include "transFile.h"

void makeWorker(factory_t *pFactory) {
  for (int i = 0; i < pFactory->threadNum; ++i) {
    pthread_create(pFactory->tidArr + i, NULL, threadFunc, (void *)pFactory);
    printf("new worker %d tid = %lu\n", i, *(pFactory->tidArr + i));
  }
}

void cleanFunc(void *pArgs) {
  factory_t *pFactory = (factory_t *)pArgs;
  pthread_mutex_unlock(&pFactory->taskQueue.mutex);
}

void *threadFunc(void *pArgs) {
  factory_t *pFactory = (factory_t *)pArgs;

  int netFd; /*为什么把 netFd 的声明放在这里，下面有解释*/

  while (1) {
    pthread_mutex_lock(&pFactory->taskQueue.mutex);

    /* 注册一个清理函数 cleanFunc，
     * 如果线程在锁定期间被取消或退出，系统会自动调用这个函数来解锁互斥锁，防止死锁。*/
    pthread_cleanup_push(cleanFunc, (void *)pFactory);

    while (pFactory->taskQueue.queueSize == 0) {
      pthread_cond_wait(&pFactory->taskQueue.cond, &pFactory->taskQueue.mutex);
    }

    printf("Get Task!\n");

    netFd = pFactory->taskQueue.pFront->netFd;
    /* int netFd = pFactory->taskQueue.pFront->netFd; */
    /* 为什么不能在这里声明 netFd？
     * 因为这行代码在 pthread_cleanup_push 和 pthread_cleanup_pop 宏之间，
     * 这两个宏之间相当于有一个花括号，如果把 netFd 声明在花括号之间，
     * 则在花括号外面的 handleEvent(netFd) 就找不到 netFd 了。
     * */
    taskDeQueue(&pFactory->taskQueue);

    /* 执行清理函数（即解锁互斥锁）。本来手动解锁互斥锁的代码现在由清理函数来负责。
     * 如果 pthread_cleanup_pop 的参数为1，清理函数会立即被执行。
     */
    pthread_cleanup_pop(1);
    /* pthread_mutex_unlock(&pFactory->taskQueue.mutex); */

    handleEvent(netFd);
    printf("pthread done! tid = %lu\n", pthread_self());
  }
}

int handleEvent(int netFd) {
  recvFile(netFd);
  close(netFd);

  return 0;
}
