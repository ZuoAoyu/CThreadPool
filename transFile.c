#include "transFile.h"

int recvn(int netFd, void *buf, int total) {
  char *p = (char *)buf;
  long cursize = 0;
  while (cursize < total) {
    ssize_t sret = recv(netFd, p + cursize, total - cursize, 0);
    if (sret == 0) {
      return 1;
    }
    cursize += sret;
  }

  return 0;
}

int recvFile(int netFd) {
  train_t t;
  bzero(&t, sizeof(t));

  // 接收 文件名长度 和 文件名
  recv(netFd, &t.dataLength, sizeof(int), 0);
  recv(netFd, t.buf, t.dataLength, 0);

  t.buf[t.dataLength] = '\0';

  // 接收文件内容
  // 先创建文件
  char filepath[100] = "temp/";
  strcat(filepath, t.buf);
  int fd = open(filepath, O_RDWR | O_CREAT, 0666); // 由 O_WRONLY 改为 O_RDWR
  ERROR_CHECK(fd, -1, "open");

  // 接收文件总大小
  off_t fileSize;
  bzero(&t, sizeof(t));
  recvn(netFd, &t.dataLength, sizeof(int));
  recvn(netFd, &fileSize, t.dataLength);
  printf("fileSize = %ld\n", fileSize);

  // 创建一个匿名管道
  int pipefds[2];
  pipe(pipefds);

  int total = 0;
  while (total < fileSize) {
    int ret = splice(netFd, NULL, pipefds[1], NULL, 4096, SPLICE_F_MORE);
    total += ret;
    splice(pipefds[0], NULL, fd, NULL, ret, SPLICE_F_MORE);
  }

  // 关闭打开的文件的 文件描述符
  close(fd);
  return 0;
}

int sendFile(int netFd) {
  train_t t = {5, "file2"};

  // 发送 文件名长度 和 文件名
  send(netFd, &t, sizeof(int) + t.dataLength, 0);

  // 打开文件
  int fd = open(t.buf, O_RDONLY);
  ERROR_CHECK(fd, -1, "open");

  // 文件状态信息
  struct stat statbuf;
  int ret = fstat(fd, &statbuf);

  // 发送文件大小
  bzero(&t, sizeof(t));
  t.dataLength = sizeof(statbuf.st_size);
  memcpy(t.buf, &statbuf.st_size, t.dataLength);
  send(netFd, &t, sizeof(int) + sizeof(off_t), MSG_NOSIGNAL);

  // 使用 sendfile 发送文件
  sendfile(netFd, fd, NULL, statbuf.st_size);

  // // 结束的时候发送一个车厢为 0 的小火车（结束标志）
  // t.dataLength = 0;
  // send(netFd, &t, sizeof(int), MSG_NOSIGNAL);

  close(fd);
  return 0;
}
