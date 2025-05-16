#include"client.h"
#include <fcntl.h>
#include <stddef.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/socket.h>
#include"socket_utils.h"
int cli_tcpinit(char* ip,char* port){
    int client_fd=socket(AF_INET,SOCK_STREAM,0);
   struct sockaddr_in serv_addr;
   memset(&serv_addr ,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   inet_pton(AF_INET,ip,&serv_addr.sin_addr);
   serv_addr.sin_port=htons(atoi(port));
   int ret =connect(client_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
   if(ret==-1){
       perror("connect failed");
       exit(EXIT_FAILURE);}
  return client_fd;
}

void  userLogin1(int sockfd,train_t* t){
   printf("userLogin1\n");
   memset(t,0,sizeof(t));
   while(1){
       printf("请输入用户名称:\n");
       scanf("%s",t->buff);
       t->type=TASK_LOGIN_SECTION1;
       t->len=strlen(t->buff)+1;
       send(sockfd,t,8+t->len,0);

      memset(t,0,sizeof(t));
       recv(sockfd,&t->len,sizeof(t->len),0);
       recv(sockfd,&t->type,sizeof(t->type),0);
       if(t->type== TASK_LOGIN_SECTION1_RESP_ERROR){
           printf("请重新输入用户名:\n");
           continue;
       }
       recv(sockfd,t->buff,t->len,0);
       break;
   }
   return ;
}

void userLogin2(int sockfd, train_t *t){
   printf("userLogin2.\n");
   while(1){
          printf("请输入密码:\n");
          char passwd[20];
          scanf("%s",passwd);
          char* encrtyped=crypt(passwd,t->buff);
          t->len=strlen(encrtyped)+1;
          t->type=TASK_LOGIN_SECTION2;
          strncpy(t->buff,encrtyped,t->len);
          send(sockfd,&t,8+t->len,0);
           
          memset(t,0,sizeof(train_t));
          recv(sockfd,&t->len,4,0);
          recv(sockfd,&t->type,4,0);
          if(t->type==TASK_LOGIN_SECTION2_RESP_ERROR){
              printf("sorry,密码不正确:\n");
              continue;
          }
          recv(sockfd,t->buff,t->len,0);
          printf("Login success:当前目录:%s\n",t->buff);
          break;
   }
   return;
}

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
        return COMMAND_MKDIR;}

        return COMMAND_NOT;
}

int frecv(int sockfd,void* buff,size_t length){
  size_t total_recv=0;
  char* ptr=(char*)buff;
  while(total_recv<length){
      size_t n=recvn(sockfd,ptr+total_recv,length-total_recv,0);

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
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret== -1){
     printf("ls任务失败，重新输入命令:\n");
    }else{
    recvn(sockfd,&length,sizeof(length),0);
    recvn(sockfd,buff,length,0);
       puts(buff);}
}

void cd_recv(int sockfd){
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret ==-1){
        printf("目录文件不存在，请重新输入: \n");

    }
    else{
        int length= -1;
        recvn(sockfd,&length,sizeof(length),0);
        char buff[1024];
       ret=recvn(sockfd,buff,length,0);
      puts(buff);
       

    }
}

void mkdir_recv(int sockfd){
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret==-1){
        printf("创建目录失败,请重新尝试: \n");
    }
}

void rmdir_recv(int sockfd){
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret== -1){
        printf("删除目录失败，请重新尝试: \n");
    }
}

void pwd_recv(int sockfd){
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret ==0){
   int length;
   char data[100];
    recvn(sockfd,&length,sizeof(length),0);
    recvn(sockfd,data,length,0);
        puts(data);
    }
    else{
        printf("获取当前目录失败，请重新尝试: \n");
    }
    
}
void gets_recv(int sockfd){
    int ret;
    recvn(sockfd,&ret,sizeof(ret),0);
    if(ret<0){
        printf("获取文件失败，请重新检查命令：\n");
    }else{
        int file_name_length;
        recvn(sockfd,&file_name_length,sizeof(file_name_length),0);
        char filename[100];
        recvn(sockfd,filename,file_name_length,0);
        int file_length;
        recvn(sockfd,&file_length,sizeof(file_length),0);
        if(file_length>104857600){
            getsbig_recv(filename,sockfd,file_length);
        }else{
            getsmall_recv(filename,sockfd,file_length);
        }
        
    }
}

void getsmall_recv(char* filename,int sockfd,int file_length){
    int file_fd=open(filename,O_RDWR|O_CREAT|O_TRUNC,0775);
    if(file_fd<0){
        perror("open");
        return;
    }
  int ret=ftruncate(file_fd,file_length);
  if(ret==0){
      char data[BUFFER_SIZE];
      int sum=0;
      int r=0;
      float percent=(float)sum/file_length*100;
      while(sum<file_length){
          r=recvn(sockfd,data,sizeof(data),0);//不能用frecv，因为第三个参数是接收最大，不满足的话会直接卡住.
          size_t bytes_wrtten=write(file_fd,data,r);
          
          sum+=bytes_wrtten;
        percent = (float)sum / file_length * 100;
        printf("\r接收文件进度: %.1f%%", percent);
        fflush(stdout);  // 刷新输出缓冲区，以便实时显示进度
      }
      if(sum== file_length){
             printf("\n文件接收完成！\n");
      }

  close(file_fd);
}}

void getsbig_recv(char* filename,int sockfd,int file_length){

}

void puts_send(train_t train,int sockfd){

    int file_fd =open(train.buff,O_RDWR);
    int ret=file_fd;
    send(sockfd,&ret,sizeof(ret),0);
    if(ret<0){
        perror("open file");
        return;
    }else{
        
    int file_name_length=strlen(train.buff)+1;
    send(sockfd,&file_name_length,sizeof(file_name_length),0);
    send(sockfd,train.buff,file_name_length,0);
    struct stat file_buff;
   fstat(file_fd,&file_buff);
   int file_length=file_buff.st_size;
   send(sockfd,&file_length,sizeof(file_length),0);
   if(file_length>104857600){
       putsbig_send(file_fd,sockfd,file_length);
           
   }else{
       putsmall_send(file_fd,sockfd,file_length);
   }
   close(file_fd);
 }
}
void putsmall_send(int fd,int sockfd,int file_length){
    char file_buff[BUFFER_SIZE];
    size_t bytes_read;
    size_t sum=0;
    float percent=(float)sum/file_length*100;
    while((bytes_read=read(fd,file_buff,sizeof(file_buff)))>0){
        if(send(sockfd,file_buff,bytes_read,0)<0){
            perror("Failed to send file data");
            return;
        }
        sum+=bytes_read;
        percent=(float)sum/file_length*100;
        printf("\r发送文件进度: %.1lf%% ",percent);
        fflush(stdout);
    }
    printf("\n");
    if(sum!=file_length){
        printf("上传文件不完善，请重新上传\n");
    }
    else{
        printf("上传文件成功\n");
    }
}

void putsbig_send(int fd,int sockfd, int file_length){

}

void client_recv(train_t train, int sockfd){
    switch(train.type){
    case COMMAND_LS:
        ls_recv(sockfd);
        break;
    case COMMAND_CD:    
        cd_recv(sockfd);
        break;
    case COMMAND_PWD:
        pwd_recv(sockfd);
        break;
    case COMMAND_MKDIR:
        mkdir_recv(sockfd);
        break;
    case COMMAND_RM:
        rmdir_recv(sockfd);
        break;
    case COMMAND_NOT:
        printf("未知命令，重新输入: \n");
        break;
    case COMMAND_GETS:
        gets_recv(sockfd);
       break;
    case COMMAND_PUTS:
       puts_send(train, sockfd);
       break;
    default:
       printf("还未产生新的操作:\n");


    }

}
size_t recvn(int sockfd,void* buff,size_t length,int num){
   int ret=recv(sockfd,buff,length,0);
   if(ret ==0){
       printf("对方已经关闭连接\n");
       close(sockfd);
       return 0;
   }else if(ret< 0){
       perror("recv failed");
       close(sockfd);
       return -1;
   }
   return ret;

}
