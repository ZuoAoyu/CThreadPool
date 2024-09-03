#ifndef __HEAD
#define __HEAD

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR_CHECK(result, num, msg)                                          \
  {                                                                            \
    if (result == num) {                                                       \
      perror(msg);                                                             \
      return -1;                                                               \
    }                                                                          \
  }

#endif
