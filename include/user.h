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

                                                      
typedef struct{                                     
     int len;                                        
     CmdType type;                                   
     char buff[1024];                                
 }train_t;     

void loginCheck1(user_info_t* user);
void loginCheck2(user_info_t* user,const char* encrypted);
void get_setting(char* setting,char* str);

#endif
