#include"client.h"
#include <fcntl.h>
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
        return COMMAND_MKDIR;}

        return COMMAND_NOT;
}

int frecv(int sockfd,void* buff,size_t length){
  size_t total_recv=0;
  char* ptr=(char*)buff;
  while(total_recv<length){
      size_t n=recv(sockfd,ptr+total_recv,length-total_recv,0);
      if(n ==0){
          printf("对方关闭连接\n");
          close(sockfd);
          return 0;
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
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret== -1){
     printf("ls任务失败，重新输入命令:\n");
    }else{
    recv(sockfd,&length,sizeof(length),0);
    recv(sockfd,buff,length,0);
       puts(buff);}
}

void cd_recv(int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret ==-1){
        printf("目录文件不存在，请重新输入: \n");

    }
    else{
        int length= -1;
        recv(sockfd,&length,sizeof(length),0);
        char buff[1024];
       ret=recv(sockfd,buff,length,0);
      puts(buff);
       

    }
}

void mkdir_recv(int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret==-1){
        printf("创建目录失败,请重新尝试: \n");
    }
}

void rmdir_recv(int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret== -1){
        printf("删除目录失败，请重新尝试: \n");
    }
}

void pwd_recv(int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret ==0){
   int length;
   char data[100];
    recv(sockfd,&length,sizeof(length),0);
    recv(sockfd,data,length,0);
        puts(data);
    }
    else{
        printf("获取当前目录失败，请重新尝试: \n");
    }
    
}
void gets_recv(int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret<0){
        printf("获取文件失败，请重新检查命令：\n");
    }else{
        int file_name_length;
        recv(sockfd,&file_name_length,sizeof(file_name_length),0);
        char filename[100];
        recv(sockfd,filename,file_name_length,0);
        int file_length;
        recv(sockfd,&file_length,sizeof(file_length),0);
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
          r=recv(sockfd,data,sizeof(data),0);//不能用frecv，因为第三个参数是接收最大，不满足的话会直接卡住.
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
    }

}

