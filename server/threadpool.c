#include <string.h>
#include <sys/epoll.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <sys/syslog.h>
#include<syslog.h>
#include<unistd.h>
#include"threadpool.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include<syslog.h>
#include <stdlib.h>
#include "jwt_token.h"
#include"ser_main.h"
#include "session.h"

void pthread_error(int ret ,char* str){

    if(ret){
        perror(str);
    exit(EXIT_FAILURE);

    }
}

int queueInit(task_queue_t* que){

    if(que){
        que->pFront = NULL;
        que->pRear = NULL;
        que->queSize = 0;
        que->flag =1;
      int ret= pthread_mutex_init(&que->mutex, NULL);
      pthread_error(ret,"muter_init failed");
      ret=pthread_cond_init(&que->cond,NULL);
      pthread_error(ret,"cond_init failed");
    }
    return 0;
}

int queueDestroy(task_queue_t* que){
 if(que){
    int ret= pthread_mutex_destroy(&que->mutex);
      pthread_error(ret,"mutex_destroy failed");
    ret =pthread_cond_destroy(&que->cond);
      pthread_error(ret,"cond_destroy failed");
   }
    return 0;
}
int queueIsEmpty(task_queue_t*  que){
    return que->queSize  == 0;
}

int taskSize(task_queue_t* que){
    return que->queSize;
}

int taskEnque(task_queue_t* que,task_t* ptask ){

    task_t * pnode = ptask;
    pnode->pNext =NULL;

   int ret=pthread_mutex_lock(&que->mutex);
   pthread_error(ret,"mutex_lock failed");

    if(queueIsEmpty(que)){
        que->pFront =pnode;
        que->pRear =pnode;
    }else{
       que->pRear =pnode;
    }
     que->queSize++;
     ret=pthread_mutex_unlock(&que->mutex);
     pthread_error(ret,"mutex_unlock failed");
     ret =pthread_cond_signal(&que->cond);
     pthread_error(ret,"cond_signal failed");
     return 0;
}

task_t* taskDeque(task_queue_t* que){
    int ret=pthread_mutex_lock(&que->mutex);
    task_t* ptask;
    while(que->flag&&queueIsEmpty(que)){
        pthread_cond_wait(&que->cond,&que->mutex);
    }
    if(que->flag){
       ptask = que->pFront;
        if(taskSize(que)==1){
            que->pFront = que->pRear =NULL;
        }else{
            que->pFront =que->pFront->pNext;
        }
        que->queSize--;

    }else{
        ptask= NULL;
    }
    ret =pthread_mutex_unlock(&que->mutex);
    pthread_error(ret,"mutex_unlock failed");
    return ptask;
}

int broadcastALL(task_queue_t* que){
    pthread_mutex_lock(&que->mutex);
    que->flag =0;
    int ret=pthread_cond_broadcast(&que->cond);
    pthread_error(ret,"broadALL failed");
    pthread_mutex_unlock(&que->mutex);
    return 0;
}

int threadpoolInit(threadpool_t* pthreadpool, int num){
   pthreadpool->pthreadNum =num ;
   pthreadpool->pthreads = calloc(num,sizeof(pthread_t));
   queueInit(&pthreadpool->que);
    
   return 0;
}

int threadpoolDestroy(threadpool_t* pthreadpool){
    
    free(pthreadpool->pthreads);
    queueDestroy(&pthreadpool->que);
    free(pthreadpool);
    return 0;
}

//线程开始
int threadpoolStart(threadpool_t* pthreadpool){

    if(pthreadpool){
        for(int i=0;i<pthreadpool->pthreadNum;i++){
            int ret=pthread_create(&pthreadpool->pthreads[i],NULL,threadFunc,pthreadpool);
            pthread_error(ret,"pthread_create failed");
        }
    } 
    return 0;
}

//线程终止
int threadpoolStop(threadpool_t * pthreadpool){
    while(!queueIsEmpty(&pthreadpool->que)){
        printf("等待任务结束，需要返回");
        sleep(1);
    }
    broadcastALL(&pthreadpool->que);
    for(int i=0;i<pthreadpool->pthreadNum;i++){
        pthread_join(pthreadpool->pthreads[i],NULL);
        printf("thread %ld join.\n",pthreadpool->pthreads[i]);
    }
    return 0;
}

//创建线程运行函数
void* threadFunc(void* arg){

    threadpool_t* pthreadpool = (threadpool_t* )arg;
    while(1){
        task_t* ptask=taskDeque(&pthreadpool->que);
        if(ptask){
            dotask(ptask);
            free(ptask);
        }else{
            break;
        }
    }
      printf("thread %ld is exiting\n",pthread_self());
      return NULL;
}

//分析消息类型
void handleMessage(int acc_fd,int epoll_fd,task_queue_t* que,MYSQL* conn){

    int cmdType=-1;
    int ret=recvn(acc_fd,epoll_fd,&cmdType,sizeof(cmdType));
    if(ret>0)
    printf("短命令池recv cmd tyoe: %s\n\n",TypeToStr(cmdType));

    int length= -1;
    ret=recvn(acc_fd,epoll_fd,&length,sizeof(length));
    if(ret>0)
    printf("短命令池recv: %d length\n",length);


    task_t* ptask= calloc(1,sizeof(task_t));
    ptask->epoll_fd=epoll_fd;
    ptask->accept_fd=acc_fd;
    ptask->type =cmdType;
    ptask->conn=conn;
    if(length>0){
        ret=recvn(acc_fd,epoll_fd,ptask->data,length);
        if(ret>0){
            if(ptask->type== COMMAND_PUTS||ptask->type== COMMAND_GETS){
                DelEpollfd(ptask->epoll_fd,ptask->accept_fd);
            }
            syslog(LOG_INFO,"短命令池操作类型：%s ,操作数据：%s 时间：%s",TypeToStr(ptask->type),ptask->data,getCurrentTime());
            taskEnque(que,ptask); } 
    }else if(length ==0){
            syslog(LOG_INFO,"短命令池操作类型：%s ,操作数据：%s 时间: %s",TypeToStr(ptask->type),ptask->data,getCurrentTime());
            taskEnque(que,ptask);
    }

}

int downpoolStart(threadpool_t* pthreadpool){

    if(pthreadpool){
        for(int i=0;i<pthreadpool->pthreadNum;i++){
            int ret=pthread_create(&pthreadpool->pthreads[i],NULL,threaddown,pthreadpool);
            pthread_error(ret,"downpthread_create failed");
        }
    } 
    return 0;
}

void* threaddown(void* arg){
    threadpool_t* pthreadpool = (threadpool_t* )arg;
    while(1){
        task_t* ptask=taskDeque(&pthreadpool->que);
        if(ptask){
            downtask(ptask);
            free(ptask);
        }else{
            break;
        }
    }
      printf("thread %ld is exiting\n",pthread_self());
      return NULL;
}

void downtask(task_t* ptask){
    switch(ptask->type){
      case COMMAND_GETS:                                                                                                                 
          getsCommand(ptask);
          addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
          break;
      case COMMAND_PUTS:
          putsCommand(ptask);
          addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
          break;
      default:
          printf("错误任务\n");
          return ;
    }
}

void server_down(int exitPipe) {
    int epoll_num = 20;

    threadpool_t* downpool = calloc(1, sizeof(threadpool_t));
    if (downpool == NULL) {
        fprintf(stderr, "downpool 分配失败");
        exit(EXIT_FAILURE);
    }

    threadpoolInit(downpool, 5);
    downpoolStart(downpool);

    openlog("myserver", LOG_PID | LOG_PERROR, LOG_LOCAL0);

    int listen_fd = ser_tcpinit("192.168.230.130", "9999");
    MYSQL* conn = mysql_db_con();
    listen(listen_fd, 10);

    printf("下载池打开连接\n");

    int epoll_fd = epoll_create1(0);
    my_error(epoll_fd, -1, "epoll_create1 failed");

    addEpollfd(epoll_fd, listen_fd, EPOLLIN);
    addEpollfd(epoll_fd, exitPipe, EPOLLIN | EPOLLET);

    struct epoll_event* events = calloc(epoll_num, sizeof(struct epoll_event));

    while (1) {
        int nready = epoll_wait(epoll_fd, events, epoll_num, -1);

        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                printf("\n\n下载池连接建立中..\n");
                int accept_fd = accept(listen_fd, NULL, NULL);
                my_error(accept_fd, -1, "accept");

                addEpollfd(epoll_fd, accept_fd, EPOLLIN | EPOLLET);
                syslog(LOG_INFO, "建立链接sockfd:%d 时间:%s ", accept_fd, getCurrentTime());
            }
            else if (fd == exitPipe) {
                printf("下载池进入退出处理\n");

                // 线程池退出
                threadpoolStop(downpool);
                threadpoolDestroy(downpool);
                close(listen_fd);
                close(epoll_fd);
                printf("\n下载池 process exit.\n");
                fflush(stdout);
                return ;
            }
            else {
                int cmdType = -1;
                int token_len=0;
                char token[256];
                recv(fd,&token_len,sizeof(token_len),0);
                recv(fd,token,token_len,0);
                Jwt_payload* user=jwt_decode(token);

                session_t* u1=session_user_by_name(user->username);
                if(u1==NULL){
                    CmdType status=COMMAND_ERROR;
                    send(fd,&status,sizeof(status),0); 
                    printf("没有该用户连接状态\n");
                    return;
                   }

                if(user==NULL){
                    CmdType status=COMMAND_ERROR;
                    send(fd,&status,sizeof(status),0); 
                    return;
                }
                    CmdType status=COMMAND_OK;
                    send(fd,&status,sizeof(status),0); 

                free_payload(user);
                
                int ret = recvn(fd, epoll_fd, &cmdType, sizeof(cmdType));
                printf("下载池recv cmd type: %s\n\n", TypeToStr(cmdType));

                int length = -1;
                ret = recvn(fd, epoll_fd, &length, sizeof(length));
                printf("下载池recv: %d length\n", length);

                task_t* ptask = calloc(1, sizeof(task_t));
                ptask->epoll_fd = epoll_fd;
                ptask->accept_fd = fd;
                ptask->type = cmdType;
                ptask->conn = conn;

                session_add(ptask->accept_fd,u1->username,u1->current_path);


                ret = recvn(fd, epoll_fd, ptask->data, length);

                    if (ret > 0) {

                        DelEpollfd(ptask->epoll_fd, ptask->accept_fd);
                        syslog(LOG_INFO, "下载池操作类型：%s ,操作数据：%s 时间：%s",
                                TypeToStr(ptask->type), ptask->data, getCurrentTime());

                        taskEnque(&downpool->que, ptask);
                       }
                }

        }
    }
}


void* server_down_thread(void* arg) {
    int exitPipeFd = *(int*)arg;  // 把 void* 转为 int
    server_down(exitPipeFd);
    return NULL;
}

