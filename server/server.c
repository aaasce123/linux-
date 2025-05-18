#include "hashtable.h"
#include"ser_main.h"
#include"threadpool.h"
#include"socket_utils.h"
#include <sys/syslog.h>
#include<syslog.h>
#include<signal.h>
#include"config.h"
#include<errno.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include<stdlib.h>
#include<stdio.h>
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
    if(argc!=2){
        fprintf(stderr,"仅导入配置文件\n");
      return 0;
    }
    

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
   HashTable ht;
   initHashTable(&ht);
   readConfig(argv[1],&ht);
   printHashTable(&ht);
   threadpool_t* pthreadpool= calloc(1,sizeof(threadpool_t));
   if(pthreadpool==NULL){
       fprintf(stderr,"pthreadpool 分配失败");
       exit(EXIT_FAILURE);
   }
   threadpoolInit(pthreadpool,atoi((const char*)find(&ht,THREAD_NUM)));
   threadpoolStart(pthreadpool);
   //打开日志文件
    openlog("myserver",LOG_PID|LOG_PERROR,LOG_LOCAL0);
   char *ip_addr=(char*)find(&ht,IP);
   char* port=(char*)find(&ht,PORT);
   int listen_fd =ser_tcpinit(ip_addr,port);

   
   listen(listen_fd,continue_wait_block);
    
   printf("打开连接\n");
     //epoll处理listen
    int epoll_fd = epoll_create1(0);
    my_error(epoll_fd,-1,"epoll_create1 failed");
    addEpollfd(epoll_fd,listen_fd,EPOLLIN);   
    addEpollfd(epoll_fd,exitPipe[0],EPOLLIN);
   struct epoll_event* events=calloc(MAX_EVENTS,sizeof(struct epoll_event));
      
      while(1){
       int nready = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
       //收到中断信号

      

         for(int i=0;i<nready;i++){
          int fd=events[i].data.fd;
          if(fd== listen_fd){
                printf("连接建立中..\n");
                int accept_fd =accept(listen_fd,NULL,NULL);
                printf("通信socket:%d\n",accept_fd);
                my_error(accept_fd,-1,"accpet  ");
                addEpollfd(epoll_fd,accept_fd,EPOLLIN|EPOLLET);
                syslog(LOG_INFO,"建立链接socfd:%d 时间:%s ",accept_fd,getCurrentTime());
         }
           else if(fd == exitPipe[0]){
               printf("进入退出处理");
               //线程池退出
               int howmany= 0;
               read(exitPipe[0],&howmany,sizeof(howmany));
               threadpoolStop(pthreadpool);
               threadpoolDestroy(pthreadpool);
               destroyHashTable(&ht);
               close(listen_fd);
               close(epoll_fd);
               close(exitPipe[0]);
               printf("\nchild process exit.\n");
               exit(0);
           }else{
              handleMessage(fd,epoll_fd,&pthreadpool->que);
              //分析任务成功
               
           }
       }
     
}} 
