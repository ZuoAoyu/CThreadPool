#include "epoll.h"
#include "factory.h"
#include "tcpInit.h"
#include "worker.h"

/* 索引 0 是读端的文件描述符，索引 1 是写端的文件描述符。 */
int exitPipe[2];

void sigFunc(int signum) {
  printf("signum = %d\n", signum);
  write(exitPipe[1], "1", 1);

  printf("Parent exit!");
}

int factoryInit(factory_t *pFactory, int threadNum) {
  bzero(pFactory, sizeof(factory_t));
  pFactory->threadNum = threadNum;
  pFactory->tidArr = (pthread_t *)calloc(threadNum, sizeof(pthread_t));
  pFactory->runningFlag = 1;

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

  pipe(exitPipe);
  if (fork() != 0) {
    // 父进程

    close(exitPipe[0]); // 关闭读端

    signal(SIGUSR1, sigFunc); // 注册信号处理函数

    wait(NULL);
    exit(0);
  }

  // 子进程
  close(exitPipe[1]); // 关闭写端

  int workerNum = atoi(argv[3]);
  factory_t factory;
  factoryInit(&factory, workerNum);

  makeWorker(&factory);

  int sockFd;
  tcpInit(argv[1], argv[2], &sockFd);

  // 初始化epoll
  int epfd = epollCreate();
  epollAdd(sockFd, epfd);
  epollAdd(exitPipe[0], epfd); // 监听读端

  int listenSize = 2; // 监听的 fd 的数量
  // （1 个 socket，加 管道的读端）

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
      } else if (readyList[i].data.fd == exitPipe[0]) {
        puts("exit threadPool!");
        factory.runningFlag = 0;

        pthread_cond_broadcast(&factory.taskQueue.cond);

        for (int j = 0; j < workerNum; ++j) {
          pthread_join(factory.tidArr[j], NULL);
        }

        puts("done");
        exit(0);
      }
    }
  }

  return 0;
}
