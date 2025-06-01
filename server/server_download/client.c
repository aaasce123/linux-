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

  char Cmd_str[COMMAND_MAX_LEN];

  while(1){
//先输入命令
    scanf("%s",Cmd_str);

    Command(Cmd_str,client_fd);
    //标出来了任务信息，
   }
  //客户端需要把不同命令种类发送的分开
  //后续放一个all_send/all_recv
}

