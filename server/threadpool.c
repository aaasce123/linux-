#include <sys/epoll.h>
#include <sys/syslog.h>
#include<syslog.h>
#include<unistd.h>
#include"threadpool.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
#include<syslog.h>
#include <stdlib.h>
#include"ser_main.h"

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
void handleMessage(int acc_fd,int epoll_fd,task_queue_t* que){
    int length= -1;
    int ret=recvn(acc_fd,epoll_fd,&length,sizeof(length));
    if(ret>0)
    printf("recv: %d length\n",length);

    int cmdType=-1;
    ret=recvn(acc_fd,epoll_fd,&cmdType,sizeof(cmdType));
    if(ret>0)
    printf("recv cmd tyoe: %s\n\n",TypeToStr(cmdType));

    task_t* ptask= calloc(1,sizeof(task_t));
    ptask->epoll_fd=epoll_fd;
    ptask->accept_fd=acc_fd;
    ptask->type =cmdType;
    if(length>0){
        ret=recvn(acc_fd,epoll_fd,ptask->data,length);
        if(ret>0){
            if(ptask->type== COMMAND_PUTS||ptask->type== COMMAND_GETS){
                DelEpollfd(ptask->epoll_fd,ptask->accept_fd);
            }
            syslog(LOG_INFO,"操作类型：%s ,操作数据：%s 时间：%s",TypeToStr(ptask->type),ptask->data,getCurrentTime());
            taskEnque(que,ptask); } 
    }else if(length ==0){
            syslog(LOG_INFO,"操作类型：%s ,操作数据：%s 时间: %s",TypeToStr(ptask->type),ptask->data,getCurrentTime());
            taskEnque(que,ptask);
    }

}

