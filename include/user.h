#ifndef USER_H
#define USER_H
#include<shadow.h>
#include"ser_main.h"
typedef enum{
    STATUS_LOGOFF,
    STATUS_LOGIN
}LoginStatus;

typedef struct{
    int sockfd;
    LoginStatus status;
    char name[20];
    char encrypted[100];
    char pwd[128];
}user_info_t;



char* Rand_salt();
void user_Register1(task_t* t);
void user_Register2(task_t* t);
void user_Login1(task_t* t);
void user_Login2(task_t* t);
void get_setting(char* setting,char* str);

#endif
