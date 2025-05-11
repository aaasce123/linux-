#ifndef CLIENT_H
#define CLIENT_H
#define _POSIX_C_SOURCE 200809L
#define BUFFER_SIZE 4096
#include<sys/types.h>
#include <netdb.h>
#include<fcntl.h>
#include<stdio.h>
#include <stddef.h>
#include <unistd.h>
typedef enum{
    REGISTER,
    LOGIN,
    COMMAND_CD,
    COMMAND_LS,
    COMMAND_PWD,
    COMMAND_PUTS,
    COMMAND_GETS,
    COMMAND_RM,
    COMMAND_MKDIR,
    COMMAND_NOT
}CmdType;

typedef struct{
    int len;
    CmdType type;
    char buff[1024];
}train_t;
CmdType Cmd_change(char* str);
int frecv(int sockfd,void* buff,size_t length);
int fsend(int sock,void* buff,size_t length);
//命令接收
void client_recv(train_t train,int sockfd );
void mkdir_recv(int sockfd);
void rmdir_recv(int sockfd);
void ls_recv(int sockfd);
void cd_recv(int sockfd);
void pwd_recv(int sockfd);
void gets_recv(int sockfd);
void getsmall_recv(char* filename,int sockfd,int file_length);
void getsbig_recv(char* filename,int sockfd,int file_length);
void written(char* data,int sockfd,int length);
#endif
