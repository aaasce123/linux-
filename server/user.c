#include"user.h"
#include "ser_main.h"
#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include "user.h"

  user_node_t node_try = {
    .user = {
        .sockfd = 0,
        .status = 0,     // 根据LoginStatus定义给个默认值
        .name = "",
        .encrypted = "",
        .pwd = ""
    },
    .next = NULL
};

int loginCheck1(user_info_t* user){
     printf("loginCheck1.\n");
     train_t t;
     memset(&t,0,sizeof(t));
     struct spwd* sp=getspnam(user->name);
     if(sp==NULL){
        t.len=0;          
        t.type=TASK_LOGIN_SECTION1_RESP_ERROR;
        send(user->sockfd,&t,8,0);
        printf("login1 faile.\n");
        return 0;
      }
     char setting[100]={0};
     strcpy(user->encrypted,sp->sp_pwdp);
     get_setting(setting,sp->sp_pwdp);
     t.len=strlen(setting)+1;
     t.type=TASK_LOGIN_SECTION1_RESP_OK;
     strncpy(t.buff,setting,t.len);
     send(user->sockfd,&t.len,sizeof(t.len),0);
     send(user->sockfd,&t.type,sizeof(t.type),0);
     send(user->sockfd,t.buff,t.len,0);
    printf("login1 success.\n");
    return 1;
}

int  loginCheck2(user_info_t* user,const char* encrypted){
    train_t t;
    memset(&t,0,sizeof(t));
    if(strcmp(user->encrypted,encrypted)==0){
        user->status=STATUS_LOGIN;
        t.type=TASK_LOGIN_SECTION2_RESP_OK;
        strcpy(t.buff,"/server");
        t.len=strlen(t.buff)+1;
        send(user->sockfd,&t,8+t.len,0);
        user->status=STATUS_LOGIN;
        strcpy(user->pwd,t.buff);
        printf("Login2 success.\n");
        return 1;
    }else{
        t.type=TASK_LOGIN_SECTION2_RESP_ERROR;
        send(user->sockfd,&t,8,0);
        printf("login2 failed.\n");
        return 0;
    }
}

void TASK_check1(task_t* ptask){
   strcpy(node_try.user.name,ptask->data);
   node_try.user.sockfd=ptask->accept_fd;
  loginCheck1(&node_try.user);
}

//写死单用户即可.
void TASK_check2(task_t* ptask){
     loginCheck2(&node_try.user,ptask->data);
     
}

user_node_t* create_node(){
user_node_t* node =(user_node_t*)calloc(1,sizeof(user_node_t));
if(!node) return NULL;
return node;
}

void insert_node(user_node_t** head, user_node_t* node){
    node->next=*head;
    *head=node;
}
void get_setting(char* setting, char* str){
  int i,j;
  for(i=0,j=0;str[i]&&j!=4;i++){
      if(str[i]=='$'){
          j++;
      }
  }
  strncpy(setting,str,i);
  setting[i]='\0';
}
