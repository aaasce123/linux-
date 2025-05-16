#ifndef SER_MAIN_H
#define SER_MAIN_H
#define _POSIX_C_SOURCE 200809L 
#include"socket_utils.h"
#include <unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<unistd.h>
#include<stdio.h>
#include <stddef.h>
#include<string.h>
#include<dirent.h>
#include <stdint.h>
#include<sys/stat.h>
#define BUFFER_SIZE 4096
typedef enum{
    COMMAND_CD,
    COMMAND_LS,
    COMMAND_PWD,
    COMMAND_PUTS,
    COMMAND_GETS,
    COMMAND_RM,
    COMMAND_MKDIR,
    TASK_LOGIN_SECTION1=100,
    TASK_LOGIN_SECTION1_RESP_OK,
    TASK_LOGIN_SECTION1_RESP_ERROR,
    TASK_LOGIN_SECTION2,
    TASK_LOGIN_SECTION2_RESP_OK,
    TASK_LOGIN_SECTION2_RESP_ERROR,
}CmdType;
//任务结点
typedef struct task_s{
CmdType type;
int epoll_fd;
 int accept_fd;
 char data[1024];
 struct task_s* pNext;
}task_t;
//任务节点，仅仅把任务信息传过去
//包括操作类型，连接端口号，操作类型带的信息;
int ser_tcpinit(char* ip, char* port);
void dotask(task_t* ptask);
int recvn(int sockfd,int epoll_fd,void* buff,size_t len);
void check_argc(int num);
void addEpollfd(int epfd, int fd,uint32_t events);
void DelEpollfd(int epfd, int fd);
void my_error(int ret, int pd ,char* str);
void pthread_error(int ret,char* str);

int frecv(int sockfd,void* buff,size_t length);
int fsend(int sockfd,void* buff,size_t length);

void Register();
void Login();

//命令
void lsCommand(task_t* ptask,int sockfd); 
//进入服务器对应目录
void cdCommand(task_t* ptask,int sockfd);
void pwdCommand(task_t* ptask,int sockfd);
void mkdirCommand(task_t* ptask, int sockfd);
void rmdirCommand(task_t* ptask, int sockfd);
//上传文件
void putsCommand(task_t* ptask, int sockfd);
void putsbig_recv(char* filename,int sockfd,int file_length);
void putsmall_recv(char* filename,int sockfd,int file_length);

//下载文件
void getsCommand(task_t* ptask,int sockfd);
void getsmallfile(int fd, int sockfd,int file_length);
void getsbigfile(task_t* ptask,int sockfd,int file_length);
#endif
