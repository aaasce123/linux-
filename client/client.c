#include "client.h"
#include <complex.h>
#include <sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include"socket_utils.h"
#include<string.h>
#define COMMAND_MAX_LEN 10

int main(int argc, char *argv[]){ 

    if(argc !=3){
        fprintf(stderr,"argc error");
        printf("\n");
        exit(EXIT_FAILURE);
    } 

  int client_fd=socket(AF_INET,SOCK_STREAM,0);

   char *ip_addr=argv[1];
   struct sockaddr_in serv_addr;
   memset(&serv_addr ,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   inet_pton(AF_INET, ip_addr,&serv_addr.sin_addr);
   serv_addr.sin_port=htons(atoi(argv[2]));

   int ret =connect(client_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
   if(ret==-1){
       perror("connect failed");
       exit(EXIT_FAILURE);}

  train_t train;
  char train_command[COMMAND_MAX_LEN];
  

  while(1){
    printf("输入开始(若终止则输入quit)\n");
    train.len=0;
    train.type=COMMAND_NOT;
    memset(train.buff,'\0',sizeof(train.buff));
//先输入命令
    scanf("%s",train_command);
    if(strcmp(train_command,"quit")==0||strcmp(train_command,"q")==0)
         break;
    train.type=Cmd_change(train_command);
    if(train.type==COMMAND_LS||train.type==COMMAND_PWD){}
    else if(train.type==COMMAND_NOT){
          printf("命令错误，请重新输入\n");
          continue;
    }
    else{  scanf("%s",train.buff);
    train.len=strlen(train.buff)+1;
    }
    //标出来了任务信息，
    send(client_fd,&train.len,sizeof(train.len),0);
    send(client_fd,&train.type,sizeof(train.type),0);
    if(train.len>0)
    send(client_fd,train.buff,train.len,0);
    int alength =-1;
    recv(client_fd,&alength,sizeof(alength),0);
    char buffa[20]={'\0'};
    frecv(client_fd,buffa,alength);
    puts(buffa);
   }
  //客户端需要把不同命令种类发送的分开
  //主要因为后面发送大文件的时候，大文件的发送格式不一样
  //还有不同命令的接收种类分开
  //方便统一处理
  //后续放一个all_send/all_recv
}
void help(){
  printf("--------------------输入指南--------------------\n");
  printf("消息长度(命令行的)");

}

