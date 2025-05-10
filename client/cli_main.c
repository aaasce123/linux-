#include"client.h"
#include <stddef.h>
#include<string.h>
#include<stdio.h>
#include <sys/socket.h>

CmdType Cmd_change(char* str){
    if(strcmp(str,"ls")==0)
        return COMMAND_LS;

    if(strcmp(str,"cd")==0)
        return COMMAND_CD;
    if(strcmp(str,"pwd")==0)
        return COMMAND_PWD;
    if(strcmp(str,"puts")==0)
        return COMMAND_PUTS;
    if(strcmp(str,"gets")==0)
        return COMMAND_GETS;
    if(strcmp(str,"rm")==0)
        return COMMAND_RM;
   if(strcmp(str,"mkdir")==0){
        return COMMAND_MADIR;}

        return COMMAND_NOT;
}

int frecv(int sockfd,void* buff,size_t length){
  size_t total_recv=0;
  char* ptr=(char*)buff;
  while(total_recv<length){
      size_t n=recv(sockfd,ptr+total_recv,length,0);
      if(n ==0){
          printf("对方关闭连接\n");
          close(sockfd);
      }else if(n<0){
          perror("recv error");
          close(sockfd);
          return -1;
      }

      total_recv+= n;
  }
  return total_recv;
}

int fsend(int sockfd,void* buff,size_t length){
        size_t total_sent=0;
        char* ptr =(char*)buff;
        while(total_sent<length){
            size_t sent=send(sockfd,ptr+total_sent,length-total_sent,0);                                                                                                                     
            if(sent ==-1){
                perror("send error");
                return -1;
            }
            total_sent +=sent;
        }       
        return total_sent;
}

void ls_recv(int sockfd){
    int length=-1;
    char buff[1024];
    recv(sockfd,&length,sizeof(length),0);
   int ret= frecv(sockfd,buff,length);
   if(ret!=length){
       printf("数据中途丢失\n,请重新输入");
   }
   else{
    printf("%s",buff);
}}
