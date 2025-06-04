#include"client.h"
#include"socket_utils.h"
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sha1.h"
int cli_tcpinit(char* ip,char* port){
    int client_fd=socket(AF_INET,SOCK_STREAM,0); struct sockaddr_in serv_addr;
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

void help(){
  printf("--------------------输入指南--------------------\n");
  printf("当前任务列表：cd，ls，mkdir，rm，pwd，get puts\n"
        "输入quit表示退出\n" );
  printf("--------------------输入指南--------------------\n");
}

void Command(char* str,int sockfd,char* token){
    if(strcmp(str,"quit")==0){
        exit(0);
   }
    if(strcmp(str,"help")==0){
        help();
    }
    CmdType type=Cmd_change(str);
    switch (type){
      case COMMAND_PWD:
         Cmd_pwd(sockfd);
          break;
      case COMMAND_LS:
          Cmd_ls(sockfd);
          break;
      case COMMAND_CD:
          Cmd_cd(sockfd);
          break;
      case COMMAND_RM:
          Cmd_rm(sockfd);
          break;
      case COMMAND_MKDIR:
          Cmd_mkdir(sockfd);
          break;
      case COMMAND_PUTS:
          Cmd_puts(token);
          break;
      case COMMAND_GETS:
          Cmd_gets(token);    
          break;
      case COMMAND_NOT:
          printf("输入的命令有误请重新输入\n");
          break;
      default:
          break;
    }
}

void Cmd_pwd(int sockfd){
  CmdType type=COMMAND_PWD;
  int len=0;
  send(sockfd,&type,sizeof(type),0);
  send(sockfd,&len,sizeof(len),0);
  
  pwd_recv(sockfd);
}

void Cmd_ls(int sockfd){
  CmdType type=COMMAND_LS;
  int len=0;
  send(sockfd,&type,sizeof(type),0);
  send(sockfd,&len,sizeof(len),0);

  ls_recv(sockfd);
}

void Cmd_cd(int sockfd){
    char buff[100];
    scanf("%s",buff);
    CmdType type=COMMAND_CD;
    int len=strlen(buff)+1;
    send(sockfd,&type,sizeof(type),0);
    send(sockfd,&len,sizeof(len),0);
    send(sockfd,buff,len,0);

    cd_recv(sockfd);

}

void Cmd_rm(int sockfd){
    char buff[100];
    scanf("%s",buff);
    CmdType type=COMMAND_RM;
    int len=strlen(buff)+1;
    send(sockfd,&type,sizeof(type),0);
    send(sockfd,&len,sizeof(len),0);
    send(sockfd,buff,len,0);

    rmdir_recv(sockfd);
}
void Cmd_mkdir(int sockfd){
    char buff[100];
    scanf("%s",buff);
    CmdType type=COMMAND_MKDIR;
    int len=strlen(buff)+1;
    send(sockfd,&type,sizeof(type),0);
    send(sockfd,&len,sizeof(len),0);
    send(sockfd,buff,len,0);

    mkdir_recv(sockfd);
    
}

void* thread_puts(void* args) {
    puts_args* arg = (puts_args*)args;

    char* buff = arg->buff;
    char* token = arg->token;

    char* filename = strrchr(buff, '/');
    if (filename != NULL) {
        filename++;
    } else {
        filename = buff;
    }
    int sockfd = cli_tcpinit("192.168.230.130", "9999");
    if (sockfd < 0) {
        printf("连接服务器失败\n");
        free(arg);
        pthread_exit(NULL);
    }

    CmdType type = COMMAND_PUTS;
    int len = strlen(buff) + 1;
    int token_len = strlen(token) + 1;

    send(sockfd, &token_len, sizeof(token_len), 0);
    send(sockfd, token, token_len, 0);

    CmdType status; 
    recv(sockfd,&status,sizeof(status),0);
    if(status==COMMAND_ERROR){
        printf("token 出现问题，请让服务器重新发放\n");
        return NULL ; 
    }

    send(sockfd, &type, sizeof(type), 0);

    send(sockfd, &len, sizeof(len), 0);
    send(sockfd, buff, len, 0);

    puts_send(filename, sockfd);

    close(sockfd);
    free(arg);  // 释放结构体内存
    pthread_exit(NULL);
}

void Cmd_puts(char* token) {
puts_args* args = (puts_args*)calloc(1, sizeof(puts_args));
if (args == NULL) {
    printf("内存分配失败\n");
    return;
}

if (scanf("%99s", args->buff) != 1) {
    printf("输入错误\n");
    free(args);
    return;
}

// 拷贝token内容，确保线程安全
strncpy(args->token, token, sizeof(args->token) - 1);
args->token[sizeof(args->token) - 1] = '\0';  // 确保终止符

pthread_t tid;
int ret = pthread_create(&tid, NULL, thread_puts, args);
if (ret != 0) {
    printf("线程创建失败，错误码：%d\n", ret);
    free(args);
    return;
}

printf("上传线程:%ld 已创建\n", tid);
pthread_detach(tid);

}

void* thread_gets(void* args) {
    gets_args* arg =(gets_args*) args;
    char* filename = arg->buff;
    char* token=arg->token;

    CmdType type = COMMAND_GETS;
    int sockfd = cli_tcpinit("192.168.230.130", "9999");
    if (sockfd < 0) {
        printf("连接服务器失败\n");
        free(args);
        pthread_exit(NULL);
    }

    int len = strlen(filename) + 1;
    int token_len=strlen(token)+1;

    send(sockfd, &token_len, sizeof(token_len), 0);
    send(sockfd, token, token_len, 0);

    CmdType status; 
    recv(sockfd,&status,sizeof(status),0);
    if(status==COMMAND_ERROR){
        printf("token 出现问题，请让服务器重新发放\n");
        return NULL ; 
    }

    send(sockfd, &type, sizeof(type), 0);

    send(sockfd, &len, sizeof(len), 0);
    send(sockfd, filename, len, 0);

    gets_recv(sockfd);

    close(sockfd);
    free(args);  // 释放动态参数内存
    pthread_exit(NULL);
}

void Cmd_gets(char* token) {
    gets_args* args = (gets_args*)calloc(1, sizeof(gets_args));
    if (args == NULL) {
        printf("内存分配失败\n");
        return;
    }

    if (scanf("%99s", args->buff) != 1) {
        printf("输入错误\n");
        free(args);
        return;
    }

    strncpy(args->token, token, sizeof(args->token) - 1);
    args->token[sizeof(args->token) - 1] = '\0';

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, thread_gets, args);
    if (ret != 0) {
        printf("线程创建失败，错误码：%d\n", ret);
        free(args);
        return;
    }

    printf("下载线程:%ld 已创建\n", tid);
    pthread_detach(tid);

}


void userRegister1(int sockfd,train_t* t,char* username){
    while(1){
    printf("请输入用户名字\n");
   scanf("%s",t->buff);
    t->len=strlen(t->buff)+1;
    t->type=TASK_REGISTER1;
    send(sockfd,&t->type,sizeof(t->type),0);
    send(sockfd,&t->len,sizeof(t->len),0);

    strncpy(username,t->buff,t->len);
    send(sockfd,t->buff,t->len,0);

    memset(t,0,sizeof(train_t));
    recv(sockfd,&t->type,sizeof(t->type),0);

   if(t->type==TASK_REGISTER1_RESP_OK){
       recv(sockfd,&t->len,sizeof(t->len),0);
       recv(sockfd,t->buff,t->len,0);
       break;   
   }
   else{
       printf("用户名已存在,请重新输入\n");
   }
  }
}
void userRegister2(int sockfd,train_t* t,char* username){
    train_t regi2=*t;
   while(1){
      *t=regi2;
      char passwd[50];
       printf("请输入密码:\n");
       scanf("%s",passwd);
      char* crypted= crypt(passwd, t->buff);
      int name_len=strlen(username)+1;
      int passwd_len=strlen(passwd)+1;
      int len=strlen(crypted)+1+name_len+passwd_len;
      CmdType status=TASK_REGISTER2;
      send(sockfd,&status,sizeof(status),0);
      send(sockfd,&len,sizeof(len),0);

      send(sockfd,username,name_len,0);
      send(sockfd,passwd,passwd_len,0);
      send(sockfd,crypted,len-name_len-passwd_len,0);

      memset(t,0,sizeof(train_t));
      recv(sockfd,&status,sizeof(status),0);
      if(status==TASK_REGISTER2_RESP_OK){
          printf("注册用户成功\n");
          break;
      }else{
        printf("注册密码不符合要求\n");
      }
   }
}

void  userLogin1(int sockfd,train_t* t,char* username){
   memset(t,0,sizeof(train_t));
   while(1){
       printf("请输入用户名称:\n");
       scanf("%s",t->buff);
       t->type=TASK_LOGIN_SECTION1;
       t->len=strlen(t->buff)+1;
      strncpy(username,t->buff,t->len);
       send(sockfd,&t->type,sizeof(t->type),0);
       send(sockfd,&t->len,sizeof(t->len),0);
       send(sockfd,t->buff,t->len,0);

      memset(t,0,sizeof(train_t));
       recv(sockfd,&t->type,sizeof(t->type),0);
       if(t->type== TASK_LOGIN_SECTION1_RESP_ERROR){
           printf("用户名不存在:\n");
           continue;
       }else{
       recv(sockfd,&t->len,sizeof(t->len),0);
       recv(sockfd,t->buff,t->len,0);
       break;}
   }
   return ;
}

void userLogin2(int sockfd, train_t* t,char* username,char* token){
   train_t pt=*t;
   while(1){
          *t=pt;
          printf("请输入密码:\n");
          char passwd[20];
          scanf("%s",passwd);
          char* encrtyped=crypt(passwd,t->buff);
          int name_len=strlen(username)+1;
          t->type=TASK_LOGIN_SECTION2;
          t->len=name_len+strlen(encrtyped)+1;
          strncpy(t->buff,encrtyped,t->len);

          send(sockfd,&t->type,sizeof(t->type),0);
          send(sockfd,&t->len,sizeof(t->len),0);
          
          send(sockfd,username,name_len,0);
          send(sockfd,t->buff,t->len-name_len,0);
           
          memset(t,0,sizeof(train_t));
          recv(sockfd,&t->type,sizeof(t->type),0);

          if(t->type==TASK_LOGIN_SECTION2_RESP_ERROR){
              printf("sorry,密码不正确:\n");
              continue;
           }else{
               memset(token,0,256);
               recv(sockfd,&t->len,sizeof(t->len),0);
               recv(sockfd,token,t->len,0);
               printf("登录成功!\n\n");
               return ;
           }
 }
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
       CmdType status;
       recvn(sockfd,&status,sizeof(status),0);
  
       if(status==COMMAND_ERROR){
           printf("ls 命令执行失败\n");
           return ;
       }
  
       if(status==COMMAND_OK){
           int len=0;
           recvn(sockfd,&len,sizeof(len),0);
           char buff[1024];
           recvn(sockfd,buff,len,0);
           puts(buff);
       }

}

void cd_recv(int sockfd){
       CmdType status;
       recvn(sockfd,&status,sizeof(status),0);
  
       if(status==COMMAND_ERROR){
           printf("cd 命令执行失败\n");
           return ;
       }
  
       if(status==COMMAND_OK){
           int len=0;
           recvn(sockfd,&len,sizeof(len),0);
           char buff[1024]={0};
           recvn(sockfd,buff,len,0);
           printf("%s:",buff);
           fflush(stdout);
       }

}

void mkdir_recv(int sockfd){
       CmdType status;
       recvn(sockfd,&status,sizeof(status),0);
  
       if(status==COMMAND_ERROR){
           printf("mkdir 命令执行失败\n");
           return ;
       }

       if(status==COMMAND_OK){
           return;
       }


}

void rmdir_recv(int sockfd){
       CmdType status;
       recvn(sockfd,&status,sizeof(status),0);
  
       if(status==COMMAND_ERROR){
           printf("rmdir 命令执行失败\n");
           return ;
       }

       if(status==COMMAND_OK){
           return;
       }
    
}

void pwd_recv(int sockfd){
    CmdType status;
    recvn(sockfd,&status,sizeof(status),0);
   
    if(status==COMMAND_ERROR){
        printf("pwd 命令执行失败\n");
        return ;
    }

    if(status==COMMAND_OK){
        int len=0;
        recvn(sockfd,&len,sizeof(len),0);
        char buff[256];
        recvn(sockfd,buff,len,0);
        printf("当前路径: %s\n",buff);
        fflush(stdout);
    }   
    
}

void gets_recv(int sockfd) {
    int status = 0;

    // 1. 接收服务端返回的状态
    if (recvn(sockfd, &status, sizeof(status), 0) <= 0) {
        perror("recv status");
        return;
    }

    if (status == COMMAND_ERROR) {
        printf("服务器拒绝传输，可能是文件不存在。\n");
        return;
    }

    // 2. 接收文件名长度
    int file_name_length = 0;
    if (recvn(sockfd, &file_name_length, sizeof(file_name_length), 0) <= 0) {
        perror("recv file_name_length");
        return;
    }

    if (file_name_length <= 0 || file_name_length >= 100) {
        fprintf(stderr, "非法的文件名长度: %d\n", file_name_length);
        return;
    }

    // 3. 接收文件名
    char filename[100] = {0};
    if (recvn(sockfd, filename, file_name_length, 0) <= 0) {
        perror("recv filename");
        return;
    }

    // 4. 接收文件长度
    int file_length = 0;
    if (recvn(sockfd, &file_length, sizeof(file_length), 0) <= 0) {
        perror("recv file_length");
        return;
    }

    if (file_length <= 0 || file_length>1024 * 1024 * 1024*1.9 ) {
        fprintf(stderr, "非法的文件大小: %d 字节\n", file_length);
        return;
    }

    printf("准备接收文件: %s (%d 字节)\n", filename, file_length);

    // 5. 选择接收方式
    if (file_length > 100 * 1024 * 1024) {
        getsbig_recv(filename, sockfd, file_length);
    } else {
        getsmall_recv(filename, sockfd, file_length);
    }

    printf("文件接收完毕: %s\n", filename);
}

void getsmall_recv(char* filename, int sockfd, int file_length) {
    int file_fd = open(filename, O_RDWR | O_CREAT, 0664);
    if (file_fd < 0) {
        perror("open");
        return;
    }

    // 获取本地已有文件大小，作为断点偏移
    struct stat st;
   if (fstat(file_fd, &st) < 0) {
        perror("fstat");
        close(file_fd);
        return;
    }
    int offset = st.st_size;

    // 通知服务器当前偏移量
    if (send(sockfd, &offset, sizeof(offset), 0) < 0) {
        perror("send offset");
        close(file_fd);
        return;
    }

    // 设置写入位置
    if (offset > 0) {
        if (lseek(file_fd, offset, SEEK_SET) < 0) {
            perror("lseek");
            close(file_fd);
            return;
        }
    } else {
        if (ftruncate(file_fd, file_length) < 0) {
            perror("ftruncate");
            close(file_fd);
            return;
        }
    }

    char buffer[BUFFER_SIZE];
    int received = offset;
    int to_receive;
    float percent;

    while (received < file_length) {
        to_receive = file_length - received;
        if (to_receive > BUFFER_SIZE) {
            to_receive = BUFFER_SIZE;
        }

        int n = recv(sockfd, buffer, to_receive, 0);
        if (n <= 0) {
            perror("recv");
            break;
        }

        int written = write(file_fd, buffer, n);
        if (written < 0) {
            perror("write");
            break;
        }

        received += written;
        percent = (float)received / file_length * 100.0f;
        printf("\r接收文件进度: %.1f%%", percent);
        fflush(stdout);
    }

    printf("\n");

    if (received != file_length) {
        fprintf(stderr, "文件接收不完整：已接收 %d / %d 字节\n", received, file_length);
    } else {
        printf("文件接收完成\n");
    }

    close(file_fd);
}

void getsbig_recv(char* filename, int sockfd, int file_length) {
    int file_fd = open(filename, O_RDWR | O_CREAT, 0664);
    if (file_fd < 0) {
        perror("open");
        return;
    }

    // 获取当前文件长度用于断点续传
    struct stat st;
    if (fstat(file_fd, &st) < 0) {
        perror("fstat");
        close(file_fd);
        return;
    }

    int local_size = st.st_size;

    // 告诉服务器我们已有的文件偏移（断点续传）
    if (send(sockfd, &local_size, sizeof(local_size), 0) < 0) {
        perror("send resume offset");
        close(file_fd);
        return;
    }

    // 设置偏移写入点
    off_t offset = (off_t)local_size;
    if (lseek(file_fd, offset, SEEK_SET) < 0) {
        perror("lseek");
        close(file_fd);
        return;
    }

    // 构造管道以支持 splice
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        close(file_fd);
        return;
    }

    int remaining = file_length - local_size;
    int total_written = local_size;
    float percent;

    while (remaining > 0) {
        int chunk_size = remaining > 65536 ? 65536 : remaining;

        // 从 socket 读取数据到管道
        int n = splice(sockfd, NULL, pipefd[1], NULL, chunk_size, 0);
        if (n <= 0) {
            perror("splice: socket -> pipe");
            break;
        }

        int to_write = n;

        // 从管道写入到文件
        while (to_write > 0) {
            int written = splice(pipefd[0], NULL, file_fd, &offset, to_write, 0);
            if (written <= 0) {
                perror("splice: pipe -> file");
                break;
            }
            to_write -= written;
            total_written += written;

            percent = (float)total_written / file_length * 100.0f;
            printf("\r接收大文件进度: %.1f%%", percent);
            fflush(stdout);
        }

        remaining -= n;
    }

    printf("\n");

    if (total_written != file_length) {
        fprintf(stderr, "文件接收不完整：已接收 %d / %d 字节\n", total_written, file_length);
    } else {
        printf("大文件接收完成\n");
    }

    close(file_fd);
    close(pipefd[0]);
    close(pipefd[1]);
}


void puts_send(char* filename, int sockfd) {

    int file_fd = open(filename, O_RDONLY);
    CmdType status;

    if (file_fd < 0) {
        perror("open file");
        status = COMMAND_ERROR;
        send(sockfd, &status, sizeof(status), 0);
        return;
    }

    // 发送成功状态
    status = COMMAND_OK;
    send(sockfd, &status, sizeof(status), 0);

    // 发送文件名长度和内容
    int file_name_length = strlen(filename) + 1;
    send(sockfd, &file_name_length, sizeof(file_name_length), 0);
    send(sockfd, filename, file_name_length, 0);

    // 计算 SHA1 并发送
    char sha1[SHA1_STR_LEN] = {0};
   file_to_sha1(filename, sha1); 
    send(sockfd, sha1, SHA1_STR_LEN - 1, 0);

    int speed;
    recv(sockfd,&speed,sizeof(speed),0);
   if(speed==1){
       printf("已完成上传\n");
       return ;
   }
    // 获取文件大小
    struct stat file_stat;
    fstat(file_fd, &file_stat);
    int file_length = file_stat.st_size;

    // 秒传判定由服务器处理，客户端只需继续等接收指令
    // 发送文件大小
    send(sockfd, &file_length, sizeof(file_length), 0);

    // 根据大小选择传输方式
    if (file_length > 100 * 1024 * 1024) {
        putsbig_send(file_fd, sockfd, file_length);
    } else {
        putsmall_send(file_fd, sockfd, file_length);
    }

}

void putsbig_send(int fd, int sockfd, int file_length) {
    int offset = 0;

    // 接收服务器返回的已接收文件长度
    if (recv(sockfd, &offset, sizeof(offset), 0) <= 0) {
        perror("recv offset from server");
        return;
    }

    off_t sent_offset = (off_t)offset;
    int remaining = file_length - offset;

    while (remaining > 0) {
        int to_send = remaining > 65536 ? 65536 : remaining;
        ssize_t sent_bytes = sendfile(sockfd, fd, &sent_offset, to_send);
        if (sent_bytes == -1) {
            perror("sendfile failed");
            return;
        }
        remaining -= sent_bytes;
    }

    if (remaining == 0) {
        printf("大文件已经全部传输，文件大小: %d 字节\n", file_length);
    }
    close(fd);
}

void putsmall_send(int fd, int sockfd, int file_length) {
    int offset = 0;

    // 接收服务器返回的当前已接收字节数（用于断点续传）
    if (recv(sockfd, &offset, sizeof(offset), 0) <= 0) {
        perror("recv offset from server");
        return;
    }

    // 如果有偏移，则设置文件读位置
    if (offset > 0) {
        if (lseek(fd, offset, SEEK_SET) < 0) {
            perror("lseek failed");
            return;
        }
    }

    char buffer[BUFFER_SIZE];
    int sum = offset;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        ssize_t bytes_sent = send(sockfd, buffer, bytes_read, 0);
        if (bytes_sent <= 0) {
            perror("send failed");
            return;
        }

        sum += bytes_sent;

        float percent = (float)sum / file_length * 100;
        printf("\r上传进度：%.1f%%", percent);
        fflush(stdout);
    }

    printf("\n");

    if (sum != file_length) {
        fprintf(stderr, "文件上传不完整：已发送 %d / %d 字节\n", sum, file_length);
    } else {
        printf("文件上传完成，总字节数: %d\n", sum);
    }

    close(fd);
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

void file_to_sha1(const char* filename,char sha1_str[SHA1_STR_LEN]){
       FILE * file=fopen(filename,"rb");
       if(file ==NULL){
           perror("file_to_sha1 fopen failed");
           sha1_str[0]='\0';
           return;
       }
       SHA_CTX ctx;
      SHA1_Init(&ctx);
    
       char buff[4096];
       size_t bytes_read;
    
       while((bytes_read=fread(buff,1,sizeof(buff),file))>0){
        if(SHA1_Update(&ctx,buff,bytes_read)!=1){
             fprintf(stderr,"SHA1_Update failed\n");
             fclose(file);
             sha1_str[0]='\0';
             return;
             }
         }
    
       //检验是否出错
       if(ferror(file)){
           perror("fread failed");
           fclose(file);
           sha1_str[0]='\0';
           return;                           
       }
       fclose(file);
    
       unsigned char sha1_digest[SHA_DIGEST_LENGTH];
      if(SHA1_Final(sha1_digest,&ctx)!=1){
           fprintf(stderr,"SHA1_Final failed\n");
           sha1_str[0]='\0';
           return;
       }
       for(int i=0;i<SHA_DIGEST_LENGTH;i++){
           sprintf(sha1_str+i*2,"%02x",sha1_digest[i]);
       }
       sha1_str[SHA1_STR_LEN-1]='\0';
    }
    


