#include"ser_main.h"
#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
void check_argc(int num){

    if(num !=3){
        fprintf(stderr,"argc error");
        exit(EXIT_FAILURE);
    } 
}

void addEpollfd(int epfd, int fd, uint32_t events){
    struct epoll_event ev;
    ev.events=events;
    ev.data.fd=fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);   
   my_error(ret,-1,"epoll_ctl failed");
}

void my_error(int ret, int pd ,char* str){
    if(ret== pd){
        perror(str);
        exit(EXIT_FAILURE);
    }
}


//任务函数进行
void dotask(task_t* ptask){
    switch(ptask->type){
    case COMMAND_LS:
        lsCommand(ptask,ptask->accept_fd);
        break;
    case COMMAND_CD:
        cdCommand(ptask,ptask->accept_fd);
        break;
    case COMMAND_PWD:
        pwdCommand(ptask,ptask->accept_fd);
        break;
    case COMMAND_MKDIR:
        mkdirCommand(ptask,ptask->accept_fd);
        break;
    case COMMAND_RM:
        rmdirCommand(ptask,ptask->accept_fd);
        break;   
    case COMMAND_GETS:
        getsCommand(ptask,ptask->accept_fd);
        break;
    } 
}

int recvn(int sockfd,int epoll_fd,void* buff,size_t len){
    int ret=recv(sockfd,buff,len,0);
    if(ret== -1){
        printf("\nrecv:%d",sockfd);
        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,NULL);
        close(sockfd);
        return -1;
    }
    if(ret== 0){
        printf("\nclient:%d close ",sockfd);
        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,NULL);
        close(sockfd);
        return 0;
    }
   return ret;
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

void cdCommand(task_t* ptask,int sockfd){
    int ret=chdir(ptask->data);
     send(sockfd,&ret,sizeof(ret),0);
    if(ret==-1){
        perror("请重新输入,失败:");
    }
    else{

    char* buff=getcwd(NULL,0);
    int length=strlen(buff)+1;    
    send(sockfd,&length,sizeof(length),0);
   send(sockfd,buff,length,0);
    free(buff);
    

}}//数据可以正常传过来，下一步需要解决不同命令
void lsCommand(task_t* ptask,int sockfd){
    
    DIR* pdir ;
    if(strlen(ptask->data)==0)
    pdir =opendir(".");
        
    int ret=-1;
    if(pdir== NULL){
        send(sockfd,&ret,sizeof(ret),0);
        perror("ls opendir failed");
    }else{
    ret=0;
    send(sockfd,&ret,sizeof(ret),0);
    struct dirent* pdirent;
    char* dirname=calloc(1024,sizeof(char));
    while((pdirent = readdir(pdir))!=NULL){
            if(pdirent->d_name[0]=='.'){
                continue;
            }
            strcat(dirname,pdirent->d_name);
            strcat(dirname,"   ");
        }
    int length=strlen(dirname)+1;//发送终止符，方便操作;
    send(sockfd,&length,sizeof(length),0);
    send(sockfd,dirname,length,0);
   free(dirname);
    }
}

void mkdirCommand(task_t* ptask,int sockfd){
    int ret=mkdir(ptask->data,0755);
    send(sockfd,&ret,sizeof(ret),0);
    if(ret ==-1){
         perror("mkdir failed");
    }
    
}

void rmdirCommand(task_t* ptask,int sockfd){
    int ret=rmdir(ptask->data);
    send(sockfd,&ret,sizeof(ret),0);
    if(ret ==-1){
        perror("rmdir failde");
    }
}


void pwdCommand(task_t* ptask,int sockfd){

    char data[100];
    int ret=0;
    if(getcwd(data,sizeof(data))!=NULL){
        int length=strlen(data)+1;
        send(sockfd,&ret,sizeof(ret),0);
        send(sockfd,&length,sizeof(length),0);
        send(sockfd,data,length,0);
    }
    else{
        ret=-1;
        send(sockfd,&ret,sizeof(ret),0);
    }
}

void getsCommand(task_t* ptask,int sockfd){
    int file_fd =open(ptask->data,O_RDONLY);
    int ret=file_fd;
    send(sockfd,&ret,sizeof(ret),0);
    if(ret<0){
        perror("open file");
    }
    else{
        int file_name_length=strlen(ptask->data)+1;
        send(sockfd,&file_name_length,sizeof(file_name_length),0);
        send(sockfd,ptask->data,file_name_length,0);
       struct stat file_buff;
       fstat(file_fd,&file_buff);
      int file_length=file_buff.st_size;
     send(sockfd,&file_length,sizeof(file_length),0);
      if(file_length>104857600){
          getsbigfile(ptask,sockfd,file_length);
      }else{
          getsmallfile(file_fd,sockfd,file_length);
      }
    }
    close(file_fd);
}

void getsmallfile(int fd,int sockfd,int file_length){
    char file_buff[BUFFER_SIZE];
    size_t bytes_read;
    size_t sum=0;
    float percent=(float)sum/file_length*100;
    while((bytes_read=read(fd,file_buff,sizeof(file_buff)))>0){
        if(fsend(sockfd,file_buff,bytes_read)<0){
              perror("Failed to send file data");
              return;
        }
        sum+=bytes_read;
        percent=(float)sum/file_length*100;
        printf("\r发送文件进度: %.1lf%%",percent);
        fflush(stdout);

    }
    printf("\n");
    if(sum!=file_length){
        printf("通知用户重新发送\n");
    }
}
void getsbigfile(task_t* ptask,int sockfd,int file_length){

}


 
