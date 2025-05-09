#include<pthread.h>
//错误处理函数
void error(int ret,int pd,char * str);
//任务结点
typedef struct task_s{
 int accept_fd;
 struct task_s* pNext;
}task_t;

//任务队列
typedef struct task_queue_s{
    task_t* pFront;
    task_t* pRear;
    int queSize;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int flag;
}task_queue_t;

//线程池
typedef struct threadpool_s{
    pthread_t* pthreads;
    int pthreadNum;
    task_queue_t que;
}threadpool_t;

//任务队列对应的操作
int queueInit(task_queue_t* que);
int queueDestroy(task_queue_t* que);
int queueIsEmpty(task_queue_t* que );
int taskSize(task_queue_t* que );
int taskEnque(task_queue_t* que,int acc_fd);
int taskDeque(task_queue_t* que );
int broadcastALL(task_queue_t* que);

//线程池对应操作
int threadpoolInit(threadpool_t* pthreadpool,int num);
int threadpoolDestroy(threadpool_t* pthreadpool);
int threadpoolStart(threadpool_t* pthreadpool);
int threadpoolStop(threadpool_t* pthreadpool);
void* threadFunc(void* arg);

