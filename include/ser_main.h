#ifndef SER_MAIN_H
#define SER_MAIN_H
#include"socket_utils.h"
#include <stddef.h>
#include<string.h>
#include<dirent.h>
#include <stdint.h>

typedef enum{
    REGISTER,
    LOGIN,
    COMMAND_CD,
    COMMAND_LS,
    COMMAND_PWD,
    COMMAND_PUTS,
    COMMAND_GETS,
    COMMAND_RM,
    COMMAND_MADIR,
    COMMAND_NOT
}CmdType;
//任务结点
typedef struct task_s{
CmdType type;
 int accept_fd;
 char data[1024];
 struct task_s* pNext;
}task_t;
//任务节点，仅仅把任务信息传过去
//包括操作类型，连接端口号，操作类型带的信息;
void dotask(task_t* ptask);
int recvn(int sockfd,int epoll_fd,void* buff,size_t len);
void check_argc(int num);
void addEpollfd(int epfd, int fd,uint32_t events);
void my_error(int ret, int pd ,char* str);
void pthread_error(int ret,char* str);

int frecv(int sockfd,void* buff,size_t length);
int fsend(int sockfd,void* buff,size_t length);
void Register();
void Login();

//命令
void* lsCommand(task_t* ptask,int sockfd); 
//进入服务器对应目录
void cdCommand(task_t* ptask,int sockfd);
void mkdirCommand();
void rmdirCommand();
//上传文件
void putsCommand();
//下载文件
void getsCommand();

#endif
