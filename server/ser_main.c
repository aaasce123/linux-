#include"ser_main.h"
#include <bits/pthreadtypes.h>
#include"user.h"
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
int  ser_tcpinit(char* ip, char* port){

   int listen_fd =socket(AF_INET,SOCK_STREAM ,0);
   struct sockaddr_in serv_addr;
   memset(&serv_addr ,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   inet_pton(AF_INET, ip,&serv_addr.sin_addr);
   serv_addr.sin_port=htons(atoi(port));
   int opt=1;
   setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
   int ret= bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
   my_error(ret,-1,"bind failed");

   return listen_fd;
}
const char* TypeToStr(CmdType cmd){
     switch(cmd) {
        case COMMAND_CD: return "COMMAND_CD";
        case COMMAND_LS: return "COMMAND_LS";
        case COMMAND_PWD: return "COMMAND_PWD";
        case COMMAND_PUTS: return "COMMAND_PUTS";
        case COMMAND_GETS: return "COMMAND_GETS";
        case COMMAND_RM: return "COMMAND_RM";
        case COMMAND_MKDIR: return "COMMAND_MKDIR";
        case TASK_LOGIN_SECTION1: return "TASK_LOGIN_SECTION1";
        case TASK_LOGIN_SECTION1_RESP_OK: return "TASK_LOGIN_SECTION1_RESP_OK";
        case TASK_LOGIN_SECTION1_RESP_ERROR: return "TASK_LOGIN_SECTION1_RESP_ERROR";
        case TASK_LOGIN_SECTION2: return "TASK_LOGIN_SECTION2";
        case TASK_LOGIN_SECTION2_RESP_OK: return "TASK_LOGIN_SECTION2_RESP_OK";
        case TASK_LOGIN_SECTION2_RESP_ERROR: return "TASK_LOGIN_SECTION2_RESP_ERROR";
        default: return "UNKNOWN_COMMAND";
    }
}

const char* getCurrentTime(){
    static char buf[25]; // 格式 "YYYY-MM-DD HH:MM:SS" 共19字符 + 1 '\0'
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    if (tm_now) {
        strftime(buf, sizeof(buf), "%F %T", tm_now);
    } else {
        buf[0] = '\0'; // 出错时返回空字符串
    }
    return buf;
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
        addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
        break;
    case COMMAND_PUTS:
        putsCommand(ptask,ptask->accept_fd);
        addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
        break;
    case TASK_LOGIN_SECTION1:
       TASK_check1(ptask); 
       break;
    case TASK_LOGIN_SECTION2:
       TASK_check2(ptask);
       break;
    default:
        printf("还未开发其他操作\n");
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
    struct stat st;
    int ret =stat(ptask->data,&st);
    if(ret ==-1){
        perror("stat failed");
        send(sockfd,&ret,sizeof(ret),0);
        return;
    }
    if(S_ISDIR(st.st_mode)){
     ret=rmdir(ptask->data);
    }else if(S_ISREG(st.st_mode)){
        ret =unlink(ptask->data);
    }else{
        fprintf(stderr,"Unspported file\n");
        ret =-1;
    }

         
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
        return;
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
          getsbigfile(file_fd,sockfd,file_length);
      }else{
          getsmallfile(file_fd,sockfd,file_length);
      }
    }
}

void getsmallfile(int fd,int sockfd,int file_length){
    int f_length;
    recv(sockfd,&f_length,sizeof(f_length),0);
    char file_buff[BUFFER_SIZE];
    size_t bytes_read;
    size_t sum=0;
    float percent=(float)sum/file_length*100;
    if(f_length==0){
        sum=0;
    }
    else{
        sum=f_length;
        lseek(fd,f_length,SEEK_SET);}
        
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
    if(sum!=file_length ){
        printf("未能传文件成功\n");
    }
    close(fd);
 }
void getsbigfile(int fd,int sockfd,int file_length){
    int f_length;
    recv(sockfd,&f_length,sizeof(f_length),0);
    off_t p_length=(off_t)f_length;
    int n=file_length-f_length;
    while(n>0){
    int r=sendfile(sockfd,fd,&p_length,n);
    if(r==-1){
        perror("sendfile failed:");
        return;
    }
    n=n-r;
    }
    if(n==0){
        printf("大文件已经全部传输\n");
    }
}


void putsCommand(task_t* ptask,int sockfd){
    int ret;
    recv(sockfd,&ret,sizeof(ret),0);
    if(ret<0){
        printf("客户端出现问题，上传文件失败\n");
        return;
    }else{
      int file_name_length=strlen(ptask->data)+1;
      recv(sockfd,&file_name_length,sizeof(file_name_length),0);
     char filename[100];
     recv(sockfd,filename,file_name_length,0);
     int file_length;
     recv(sockfd,&file_length,sizeof(file_length),0);
     if(file_length>104857600){
         putsbig_recv(filename,sockfd,file_length);
     }else{
         putsmall_recv(filename,sockfd,file_length);
     }
    }
}

void putsmall_recv(char* filename,int sockfd,int file_length){
    int file_fd=open(filename,O_RDWR|O_CREAT,0755);
       if(file_fd<0){                                                
           perror("open");                                           
           return;                                                   
       } 
       struct stat file_buff;
       fstat(file_fd,&file_buff);
       int f_length=file_buff.st_size;
       send(sockfd,&f_length,sizeof(f_length),0);

         char data[BUFFER_SIZE];
         int sum=0;
         int r;
         float percent=(float)sum/file_length*100;                   
         if(f_length==0){
             ftruncate(file_fd,file_length);
             sum=0;
         }else{
             sum=f_length;
             lseek(file_fd,f_length,SEEK_SET);
         }
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
}


 
void putsbig_recv(char* filename,int sockfd,int file_length){
        int file_fd=open(filename,O_RDWR|O_CREAT,0775);
          if(file_fd<0){
              perror("open");
              return;
          }
          struct stat file_buff;
          fstat(file_fd,&file_buff);
          int f_length=file_buff.st_size;
          send(sockfd,&f_length,sizeof(f_length),0);                                                                            
          
         int pipefd[2];
         pipe(pipefd);
         int n=file_length-f_length;
         off_t offset=f_length;
         while(n>0){
         int splice_move=n>65536? 65536:n;
         int r=splice(sockfd,NULL,pipefd[1],NULL,splice_move,SPLICE_F_MORE);
           if(r==-1){
               perror("splice sockfd->pipe");
               break;
             }
           int to_write=r;
             while(to_write>0){
             int w=splice(pipefd[0],NULL,file_fd,&offset,to_write,SPLICE_F_MORE);
             if(w==-1)
             {
                 perror("spilce pipe->file");
                 break;
             }
             to_write-=w;
          }
             if(r==0){
              break;
             }
             n-=r;
         }
         close(pipefd[0]);
         close(pipefd[1]);
         close(file_fd);
         if(n==0){
             printf("文件全部接收\n");
         }
         else{
             printf("文件传输未完成，剩余%d 字节 \n",n);
       }

}

void DelEpollfd(int epfd,int fd){
    if(epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL)==-1){
        perror("epoll DEL failed ");
 }
}

