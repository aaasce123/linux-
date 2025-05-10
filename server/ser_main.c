#include"ser_main.h"
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
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
    if(ptask->type==REGISTER){
//register();
    } 
    if(ptask->type== LOGIN){
        //login();
    }
    if(ptask->type==COMMAND_LS ){
        //命令行();;

        lsCommand(ptask,ptask->accept_fd);
        printf("执行命令行\n");
    }
    if(ptask->type==COMMAND_CD){
        cdCommand(ptask,ptask->accept_fd);
    }
       
}

int recvn(int sockfd,int epoll_fd,void* buff,size_t len){
    int ret=recv(sockfd,buff,len,0);
    if(ret== -1){
        printf("recv:%d",sockfd);
        perror(" failed");
        epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,NULL);
        close(sockfd);
        return -1;
    }
    if(ret== 0){
        printf("client:%d close error",sockfd);
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

void cdCommand(task_t* ptask,int sockfd){
    int ret=chdir(ptask->data);
    printf("%d\n",ret);
    char* buff=getcwd(NULL,0);
    int length=strlen(buff)+1;    
    send(sockfd,&length,sizeof(length),0);
   fsend(sockfd,buff,length);

    puts(buff);
    free(buff);

}//数据可以正常传过来，下一步需要解决不同命令
void* lsCommand(task_t* ptask,int sockfd){
    
    DIR* pdir ;
    if(strlen(ptask->data)==0)
    pdir =opendir(".");
        
    if(pdir== NULL){
        perror("ls opendir failed");
       return NULL;
    }
    struct dirent* pdirent;
    char* dirname=calloc(1024,sizeof(char));
    while((pdirent = readdir(pdir))!=NULL){
            if(pdirent->d_name[0]=='.'){
                continue;
            }
            strcat(dirname,pdirent->d_name);
            strcat(dirname,"   ");
        }
    strcat(dirname,"\n");
    int length=strlen(dirname)+1;//发送终止符，方便操作;
    send(sockfd,&length,sizeof(length),0);
    fsend(sockfd,dirname,length);
   free(dirname);
      return NULL;
    }








 
