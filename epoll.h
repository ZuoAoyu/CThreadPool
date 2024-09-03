#ifndef __EPOLL
#define __EPOLL

#include "head.h"

int epollCreate();
int epollAdd(int fd, int epfd);
int epollDel(int fd, int epfd);

#endif
