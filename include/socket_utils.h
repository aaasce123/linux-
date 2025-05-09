#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdio.h>      // 标准输入输出函数，如 printf, perror
#include <stdlib.h>     // 标准库，如 exit, malloc
#include <string.h>     // 字符串处理函数，如 memset, strcpy
#include <unistd.h>     // Unix 标准函数，如 close, read, write
#include<sys/epoll.h>
#include <sys/types.h>  // 基本数据类型，如 pid_t, size_t
#include <sys/socket.h> // socket, bind, connect, recv, send 等函数
#include <netinet/in.h> // sockaddr_in 结构体，IPPROTO_TCP 等
#include <arpa/inet.h>  // inet_pton, inet_ntop 等 IP 地址转换函数
#include <netdb.h>      // 域名解析，如 gethostbyname
#include<sys/wait.h>
#include <errno.h>      // errno 错误处理宏

#endif // SOCKET_UTILS_H

