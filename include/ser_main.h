#ifndef SER_MAIN_H
#define SER_MAIN_H
#define _POSIX_C_SOURCE 200809L 
#define _GNU_SOURCE 
#include"socket_utils.h"
#include<sys/sendfile.h>
#include"sha1.h"
#include <unistd.h>
#include<time.h>
#include<fcntl.h>
#include<sys/types.h>
#include<mysql/mysql.h>
#include"session.h"
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

    TASK_REGISTER1=200,
    TASK_REGISTER1_RESP_OK,
    TASK_REGISTER1_RESP_ERROR,
    TASK_REGISTER2,
    TASK_REGISTER2_RESP_OK,
    TASK_REGISTER2_RESP_ERROR,

    COMMAND_ERROR,
    COMMAND_OK,
}CmdType;
//任务结点
typedef struct task_s{
 CmdType type;
 MYSQL* conn;
 int epoll_fd;
 int accept_fd;
 char data[256];
 struct task_s* pNext;
}task_t;
//任务节点，仅仅把任务信息传过去
//包括操作类型，连接端口号，操作类型带的信息;
const char* TypeToStr(CmdType cmd);
const char* getCurrentTime();
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

//命令
void pwdCommand(task_t* ptask);
void lsCommand(task_t* ptask); 
//进入服务器对应目录
void cdCommand(task_t* ptask);
void mkdirCommand(task_t* ptask);
void removeCommand(task_t* ptask);

void server_down();
//上传文件
void putsCommand(task_t* ptask);
int putsbig_recv(char* filename,session_t* user,int file_length);
int  putsmall_recv(char* filename,session_t* user,int file_length);

void insert_file_metadata(task_t* ptask,char* virtual, char* full_path,const char* sha1, const char* filename,int file_length);
int check_file_exists(const char* sha1,task_t* patsk);
void insert_file_metadata_speed(task_t* ptask,char* filename,char* virtual, char* sha1);
//下载文件
void getsCommand(task_t* ptaskd);
void getsmallfile(int fd, int sockfd,int file_length);
void getsbigfile(int fd,int sockfd,int file_length);

void server_down(int exitPipe);
#endif
