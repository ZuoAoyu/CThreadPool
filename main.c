#include "epoll.h"
#include "factory.h"
#include "tcpInit.h"
#include "worker.h"

int factoryInit(factory_t *pFactory, int threadNum) {
  bzero(pFactory, sizeof(factory_t));
  pFactory->threadNum = threadNum;
  pFactory->tidArr = (pthread_t *)calloc(threadNum, sizeof(pthread_t));

  bzero(&pFactory->taskQueue, sizeof(taskQueue_t));
  pthread_cond_init(&pFactory->taskQueue.cond, NULL);
  pthread_mutex_init(&pFactory->taskQueue.mutex, NULL);

  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    fprintf(stderr, "args failed.\n");
    fprintf(stderr, "usage: ./server 127.0.0.1 2338 10");
    exit(-1);
  }

  int workerNum = atoi(argv[3]);
  factory_t factory;
  factoryInit(&factory, workerNum);

  makeWorker(&factory);

  int sockFd;
  tcpInit(argv[1], argv[2], &sockFd);

  // 初始化epoll
  int epfd = epollCreate();
  epollAdd(sockFd, epfd);

  int listenSize = 1; // 监听的 fd 的数量
  // （1 个 socket）

  struct epoll_event *readyList =
      (struct epoll_event *)calloc(listenSize, sizeof(struct epoll_event));

  while (1) {
    int readyNum = epoll_wait(epfd, readyList, listenSize, -1);
    for (int i = 0; i < readyNum; ++i) {
      if (readyList[i].data.fd == sockFd) {
        puts("accept ready");

        int netFd = accept(sockFd, NULL, NULL);

        pthread_mutex_lock(&factory.taskQueue.mutex);

        taskEnQueue(&factory.taskQueue, netFd);
        printf("New Task\n");
        pthread_cond_signal(&factory.taskQueue.cond);

        pthread_mutex_unlock(&factory.taskQueue.mutex);
      }
    }
  }

  return 0;
}
