#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <unistd.h>     // Unix 标准函数，如 close, read, write
#include<sys/epoll.h>
#include <sys/types.h>  // 基本数据类型，如 pid_t, size_t
#include <sys/socket.h> // socket, bind, connect, recv, send 等函数
#include <netinet/in.h> // sockaddr_in 结构体，IPPROTO_TCP 等
#include <arpa/inet.h>  // inet_pton, inet_ntop 等 IP 地址转换函数
#include <netdb.h>      // 域名解析，如 gethostbyname
#include<sys/wait.h>

#endif // SOCKET_UTILS_H

