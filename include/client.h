#ifndef CLIENT_H
#define CLIENT_H
#define _POSIX_C_SOURCE 200809L
#define BUFFER_SIZE 4096
#define _GNU_SOURCE
#define SHA1_STR_LEN 41                              
 #include <openssl/sha.h>
#include<sys/types.h>
#include<sys/sendfile.h>
#include <netdb.h>
#include<fcntl.h>
#include<crypt.h>
#include<stdio.h>
#include<sys/stat.h>
#include <stddef.h>
#include <unistd.h>
typedef enum{
    COMMAND_CD,
    COMMAND_LS,
    COMMAND_PWD,
    COMMAND_PUTS,
    COMMAND_GETS,
    COMMAND_RM,
    COMMAND_MKDIR,
    COMMAND_NOT,
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

typedef struct{
    int len;
    CmdType type;
    char buff[1024];
}train_t;
int cli_tcpinit(char* ip,char* port);
void help();

CmdType Cmd_change(char* str);

void Command(char* str,int sockfd);
void Cmd_pwd(int sockfd);
void Cmd_ls(int sockfd);
void Cmd_cd(int sockfd);
void Cmd_rm(int sockfd); 
void Cmd_mkdir(int sockfd);
void Cmd_puts(int sockfd);
void Cmd_gets(int sockfd);


int frecv(int sockfd,void* buff,size_t length);
int fsend(int sock,void* buff,size_t length);

//命令接收
void userLogin1(int sockfd,train_t* train,char* username);
void userLogin2(int sockfd,train_t* train,char* username);
void userRegister1(int sockfd,train_t* train,char* username);
void userRegister2(int sockfd,train_t* train,char* username);

void mkdir_recv(int sockfd);
void rmdir_recv(int sockfd);
void ls_recv(int sockfd);
void cd_recv(int sockfd);
void pwd_recv(int sockfd);

void gets_recv(int sockfd);
void getsmall_recv(char* filename,int sockfd,int file_length);
void getsbig_recv(char* filename,int sockfd,int file_length);

void puts_send(char* filename,int sockfd);
void putsmall_send(int fd,int sockfd,int file_length);
void putsbig_send(int fd,int sockfd, int file_length);
size_t recvn(int sockfd,void* buff, size_t lengtg,int nume);


#endif
