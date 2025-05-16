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

typedef struct user_node{
    user_info_t user;
    struct user_node* next;
}user_node_t;                                                      

typedef struct{                                     
     int len;                                        
     CmdType type;                                   
     char buff[1024];                                
 }train_t;     

extern  user_node_t node_try;
int loginCheck1(user_info_t* user);
int loginCheck2(user_info_t* user,const char* encrypted);
void TASK_check1(task_t* ptask);
void TASK_check2(task_t* ptask);
void get_setting(char* setting,char* str);

user_node_t* create_node();
void insert_node(user_node_t** head , user_node_t* node);
#endif
