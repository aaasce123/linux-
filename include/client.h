#ifndef CLIENT_H
#define CLIENT_H
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
    COMMAND_MADIR,
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
void client_recv(train_t train);
void ls_recv(int sockfd);
#endif
