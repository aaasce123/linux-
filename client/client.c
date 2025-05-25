#include "client.h"
#include <complex.h>
#include <math.h>
#include <sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include"socket_utils.h"
#include<string.h>
#define COMMAND_MAX_LEN 100
void help();

int main(int argc, char *argv[]){ 


  int client_fd=cli_tcpinit("192.168.230.130","8080");
  train_t train;
  char username[99];
  int choice=0;

  while(choice!=3){
  printf("1登录,2注册,3退出\n");
  scanf("%d",&choice);
  if(choice==1){
      userLogin1(client_fd,&train,username);
      userLogin2(client_fd,&train,username);
      break;
  }
  if(choice==2){
  userRegister1(client_fd,&train,username);
  userRegister2(client_fd,&train,username);
  }
}

  char train_command[COMMAND_MAX_LEN];
  help();

  while(1){
    train.len=0;
    train.type=COMMAND_NOT;
    memset(train.buff,'\0',sizeof(train.buff));
//先输入命令
    scanf("%s",train_command);
    if(strcmp(train_command,"quit")==0||strcmp(train_command,"q")==0){
        return 0;
    }
    if(strcmp(train_command,"help")==0){
        help();
        continue;
    }

    train.type=Cmd_change(train_command);

    if(train.type==COMMAND_LS||train.type==COMMAND_PWD){

    }
    else if(train.type==COMMAND_NOT){
          printf("命令错误，请重新输入\n");
          continue;
    }
    else{ 

    scanf("%s",train.buff);
    train.len=strlen(train.buff)+1;
    }
    //标出来了任务信息，
   fsend(client_fd,&train.len,sizeof(train.len));
   fsend(client_fd,&train.type,sizeof(train.type));
    if(train.len>0)
   fsend(client_fd,train.buff,train.len);
    client_recv(train,client_fd);
   }
  //客户端需要把不同命令种类发送的分开
  //后续放一个all_send/all_recv
}
void help(){
  printf("--------------------输入指南--------------------\n");
  printf("当前任务列表：cd，ls，mkdir，rm，pwd，get puts\n"
        "输入quit表示退出\n" );


  printf("--------------------输入指南--------------------\n");

}

