#include"user.h"
#include "ser_main.h"
#include<stdio.h>
#include<string.h>
void loginCheck1(user_info_t* user){
     printf("loginCheck1.\n");
     train_t t;
     int ret;
     memset(&t,0,sizeof(t));
     struct spwd* sp=getspnam(user->name);
     if(sp==NULL){
        t.len=0;          
        t.type=TASK_LOGIN_SECTION1_RESP_ERROR;
        send(user->sockfd,&t,8,0);
        printf("login1 faile.\n");
      }
     char setting[100]={0};
     strcpy(user->encrypted,sp->sp_pwdp);
     get_setting(setting,sp->sp_pwdp);
     t.len=strlen(setting)+1;
     t.type=TASK_LOGIN_SECTION1_RESP_OK;
     strncpy(t.buff,setting,t.len);
     send(user->sockfd,&t,8+t.len,0);
    printf("login1 success.\n");
}

void loginCheck2(user_info_t* user,const char* encrypted){
    train_t t;
    memset(&t,0,sizeof(t));
    if(strcmp(user->encrypted,encrypted)==0){
        user->status=STATUS_LOGIN;
        t.type=TASK_LOGIN_SECTION2_RESP_OK;
        strcpy(t.buff,"/server");
        t.len=strlen(t.buff)+1;
        send(user->sockfd,&t,8+t.len,0);
        printf("Login2 success.\n");
    }else{
        t.type=TASK_LOGIN_SECTION2_RESP_ERROR;
        send(user->sockfd,&t,8,0);
        printf("login2 failed.\n");
    }
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
