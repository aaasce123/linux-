#include"main.h"
#include "socket_utils.h"
#include<signal.h>
#include <pthread.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#define continue_wait_block 10
#define MAX_EVENTS 20
#define PTHREAD_NUM 8

int exitPipe[2];
void sigHandler(int num){
    printf("\n sig is coming.\n");
    int one= 1;
    write(exitPipe[1],&one,sizeof(one));
}

int main(int  argc, char *argv[]){

   check_argc(argc);  
    
   pipe(exitPipe);
   pid_t pid=fork();

   if(pid>0 ){
    close(exitPipe[0]);
    signal(SIGUSR1,sigHandler);
     wait(NULL);
     
     close(exitPipe[1]);
     printf("\nparent process exit;.\n");
     exit(0);
   }

   close(exitPipe[1]);
   //启动线程池;
   threadpool_t* pthreadpool= calloc(1,sizeof(threadpool_t));
   if(pthreadpool==NULL){
       fprintf(stderr,"pthreadpool 分配失败");
       exit(EXIT_FAILURE);
   }
   threadpoolInit(pthreadpool,PTHREAD_NUM);
   threadpoolStart(pthreadpool);

   int listen_fd =socket(AF_INET,SOCK_STREAM ,0);
   char *ip_addr=argv[1];
   struct sockaddr_in serv_addr;
   memset(&serv_addr ,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   inet_pton(AF_INET, ip_addr,&serv_addr.sin_addr);
   serv_addr.sin_port=htons(atoi(argv[2]));
   
   int opt=1;
   setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
   int ret= bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   error(ret,-1,"bind failed");
   
   listen(listen_fd,continue_wait_block);
    
   printf("打开连接\n");
     //epoll处理listen
    int epoll_fd = epoll_create1(0);
    error(epoll_fd,-1,"epoll_create1 failed");
    addEpollfd(epoll_fd,listen_fd,EPOLLIN);   
    addEpollfd(epoll_fd,exitPipe[0],EPOLLIN);
   struct epoll_event* events=calloc(MAX_EVENTS,sizeof(struct epoll_event));
      
      while(1){
       int nready = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
       //收到中断信号
       if(nready==-1 && errno ==EINTR){
           continue;
           }

       else if(nready== -1){
           error(nready,-1,"epoll_wait");
       }else{

         for(int i=0;i<nready;i++){
          int fd=events[i].data.fd;
          if(fd== listen_fd){
                printf("连接建立中..\n");
                int accept_fd =accept(listen_fd,NULL,NULL);
                printf("%d\n",accept_fd);
                error(accept_fd,-1,"accpet  ");
                taskEnque(&pthreadpool->que,accept_fd);
         }
           else if(fd == exitPipe[0]){
               //线程池退出
               int howmany= 0;
               read(exitPipe[0],&howmany,sizeof(howmany));
               threadpoolStop(pthreadpool);
               threadpoolDestroy(pthreadpool);
               close(listen_fd);
               close(epoll_fd);
               close(exitPipe[0]);
               printf("\nchild process exit.\n");
               exit(0);
           }
       }
     }
}} 
