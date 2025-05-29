#include"ser_main.h"
#include <bits/pthreadtypes.h>
#include "session.h"
#include "sha1.h"
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
        case TASK_REGISTER1: return "TASK_REGISTER1";
        case TASK_REGISTER1_RESP_ERROR: return"TASK_REGISTER1_RESP_ERROR";
        case TASK_REGISTER1_RESP_OK: return"TASK_REGISTER1_RESP_OK";
        case TASK_REGISTER2:  return"TASK_REGISTER2";
        case TASK_REGISTER2_RESP_ERROR:return "TASK_REGISTER2_RESP_ERROR";
        case TASK_REGISTER2_RESP_OK: return "TASK_REGISTER2_RESP_OK";
        default: return "UNKNOWN_COMMAND";
    }
}

const char* getCurrentTime(){
    static char buf[25]; // 格式 "YYYY-MM-DD HH:MM:SS" 共19字符 + 1 '\0'
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now); if (tm_now) {
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
            lsCommand(ptask);
            break;
        case COMMAND_CD:
            cdCommand(ptask);
            break;
        case COMMAND_PWD:
           pwdCommand(ptask);
            break;
        case COMMAND_MKDIR:
            mkdirCommand(ptask);
            break;
        case COMMAND_RM:
            removeCommand(ptask);
            break;   
        case COMMAND_GETS:
            getsCommand(ptask);
            addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
            break;
        case COMMAND_PUTS:
            putsCommand(ptask);
            addEpollfd(ptask->epoll_fd,ptask->accept_fd,EPOLLIN|EPOLLET);
            break;
        case TASK_LOGIN_SECTION1:
            user_Login1(ptask);
           break;
        case TASK_LOGIN_SECTION2:
            user_Login2(ptask);
           break;
        case TASK_REGISTER1:
           user_Register1(ptask);
           break;
        case TASK_REGISTER2:
            user_Register2(ptask);
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
           session_remove(sockfd); 
            close(sockfd);
            return -1;
        }
        if(ret== 0){
            printf("\nclient:%d close ",sockfd);
            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,sockfd,NULL);
           session_remove(sockfd); 
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
              session_remove(sockfd); 
               close(sockfd);
               return 0;
           }else if(n<0){
               perror("recv error");
              session_remove(sockfd); 
               close(sockfd);
               return -1;
           }

           total_recv+= n;
       }
        return total_recv;
      }

    void cdCommand(task_t* ptask){
        session_t* user=session_user(ptask->accept_fd);
         if (user == NULL) {
            fprintf(stderr, "cdCommand 未找到会话信息\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

         char new_virtual_path[512]={0};
         if (strcmp(ptask->data, "..") == 0) {
        // 退回上一级路径
        strncpy(new_virtual_path, user->current_path, sizeof(new_virtual_path) - 1);

        // 去掉最后一个 '/' 及其后内容
        char* last_slash = strrchr(new_virtual_path, '/');
        if (last_slash != NULL && last_slash != new_virtual_path) {
            *last_slash = '\0';
        } else {
            // 已经在根目录或路径格式异常，回到根目录
            strcpy(new_virtual_path, "");
        }
    } else if (ptask->data[0] == '/') {
        snprintf(new_virtual_path, sizeof(new_virtual_path), "%s", ptask->data);
    } else {
        snprintf(new_virtual_path, sizeof(new_virtual_path), "%s/%s", user->current_path, ptask->data);
    }

    // 路径规范化，消除重复斜杠等问题
           normalize_path(new_virtual_path);
             char real_path[1024];
             snprintf(real_path, sizeof(real_path), "/home/cccbiji/linux-/server/user_people%s", new_virtual_path);

             DIR* dir=opendir(real_path);
             if(dir== NULL){

                 CmdType status=COMMAND_ERROR;
                 send(ptask->accept_fd,&status,sizeof(status),0);

             }else{
                 closedir(dir);
                 if(ptask->data[0]=='/'){
                 strncpy(user->current_path,ptask->data , sizeof(user->current_path) - 1);
                 }
                 else{

                 strncpy(user->current_path, new_virtual_path, sizeof(user->current_path) - 1);

                 }

                  char sql[1024] = {0};
                  snprintf(sql, sizeof(sql),
                          "UPDATE user SET pwd='%s' WHERE username='%s'",
                               user->current_path, user->username);

               if (mysql_query(ptask->conn, sql) != 0) {
                  fprintf(stderr, "更新数据库失败: %s\n", mysql_error(ptask->conn));
                  CmdType status = COMMAND_ERROR;
                  send(ptask->accept_fd, &status, sizeof(status), 0);
                  return;
                    }

                 CmdType status = COMMAND_OK;
                 send(ptask->accept_fd,&status,sizeof(status),0);
                 int len=strlen(user->current_path)+1;
                 send(ptask->accept_fd,&len,sizeof(len),0);
                 send(ptask->accept_fd,user->current_path,len,0);
             }

    }

    void lsCommand(task_t* ptask) {
        session_t* user = session_user(ptask->accept_fd);
        if (user == NULL) {
            fprintf(stderr, "lsCommand: 无法获取用户会话\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 使用当前路径，无需从 ptask->data 获取参数
         char* virtual_path = user->current_path;

        int id = get_dir_id((char*)virtual_path, ptask->conn);
        if (id < 0) {
            fprintf(stderr, "lsCommand: 获取 id 失败\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 获取用户 ID
        int owner_id = get_owner_id(user->username, ptask->conn);
        if (owner_id <= 0) {
            fprintf(stderr, "lsCommand: 获取 owner_id 失败\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 查询文件表，找出当前目录下的所有文件和子目录
        char sql[512];
        snprintf(sql, sizeof(sql),
                 "SELECT filename FROM file_table WHERE parent_id = %d AND owner_id = %d ;",
                 id, owner_id);

        if (mysql_query(ptask->conn, sql) != 0) {
            fprintf(stderr, "lsCommand: 数据库查询失败: %s\n", mysql_error(ptask->conn));
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        MYSQL_RES* result = mysql_store_result(ptask->conn);
        if (result == NULL) {
            fprintf(stderr, "lsCommand: 获取查询结果失败: %s\n", mysql_error(ptask->conn));
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 拼接返回数据
        char buffer[2048] = {0};
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result)) != NULL) {
            strncat(buffer, row[0], sizeof(buffer) - strlen(buffer) - 1);
            strncat(buffer, "    ", sizeof(buffer) - strlen(buffer) - 1);
        }
        mysql_free_result(result);

        // 发送结果
        CmdType status = COMMAND_OK;
        send(ptask->accept_fd, &status, sizeof(status), 0);

        int len = strlen(buffer) + 1;
        send(ptask->accept_fd, &len, sizeof(len), 0);
        send(ptask->accept_fd, buffer, len, 0);
    }


    void mkdirCommand(task_t* ptask) {
        session_t* user = session_user(ptask->accept_fd);
        if (user == NULL) {
            fprintf(stderr, "mkdirCommand: 会话获取失败\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 构造虚拟路径 new_virtual_path
        char new_virtual_path[512] = {0};
        if (ptask->data[0] == '/') {
            snprintf(new_virtual_path, sizeof(new_virtual_path), "%s", ptask->data + 1);
        } else {
            snprintf(new_virtual_path, sizeof(new_virtual_path), "%s/%s", user->current_path, ptask->data);
        }

        // 构造真实路径 real_path
        char real_path[1024] = {0};
        snprintf(real_path, sizeof(real_path), "/home/cccbiji/linux-/server/user_people%s", new_virtual_path);

        // 创建目录
        int ret = mkdir(real_path, 0755);
        if (ret == -1) {
            perror("mkdir");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 获取目录名（路径最后一段）
        const char* filename = strrchr(new_virtual_path, '/');
        if (filename != NULL) {
            filename++;
        } else {
            filename = new_virtual_path;
        }

        // 获取 parent_id
        int parent_id = get_parent_id(new_virtual_path, ptask->conn);
        if (parent_id < 0) {
            fprintf(stderr, "mkdirCommand: 获取 parent_id 失败\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 获取 owner_id
        int owner_id = get_owner_id(user->username, ptask->conn);
        if (owner_id <= 0) {
            fprintf(stderr, "mkdirCommand: 获取 owner_id 失败\n");
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        char sha1[SHA1_STR_LEN] = {0};

        // 获取文件大小和类型
        int filesize =0 ;            //  0
        const char* filetype = get_file_type(real_path);    // 返回 "dir"

        // 插入数据库
        char sql[1024] = {0};
        snprintf(sql, sizeof(sql),
            "INSERT INTO file_table (parent_id, filename, owner_id, sha1, filesize, type) "
            "VALUES (%d, '%s', %d, '%s', %d, '%s');",
            parent_id,
            filename,
            owner_id,
            sha1,
            filesize,
            filetype
        );

        if (mysql_query(ptask->conn, sql) != 0) {
            fprintf(stderr, "mkdirCommand: 插入数据库失败: %s\n", mysql_error(ptask->conn));
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        // 最终响应客户端
        CmdType status = COMMAND_OK;
        send(ptask->accept_fd, &status, sizeof(status), 0);
    }


void removeCommand(task_t* ptask) {
    session_t* user = session_user(ptask->accept_fd);
    if (!user) {
        fprintf(stderr, "session 获取失败\n");
        CmdType status = COMMAND_ERROR;
        send(ptask->accept_fd, &status, sizeof(status), 0);
        return;
    }

    // 构造虚拟路径
    char virtual_path[512] = {0};
    if (ptask->data[0] == '/') {
        snprintf(virtual_path, sizeof(virtual_path), "%s", ptask->data);
    } else {
        snprintf(virtual_path, sizeof(virtual_path), "%s/%s", user->current_path, ptask->data);
    }

    // 构造真实路径
    char real_path[1024] = {0};
    snprintf(real_path, sizeof(real_path), "/home/cccbiji/linux-/server/user_people%s", virtual_path);

    // 获取类型：文件 or 目录
    const char* type = get_file_type(real_path);
    if (!type) {
        fprintf(stderr, "无法识别类型或路径不存在: %s\n", real_path);
        CmdType status = COMMAND_ERROR;
        send(ptask->accept_fd, &status, sizeof(status), 0);
        return;
    }

    // 尝试删除文件或目录
    int ret = -1;
    if (strcmp(type, "普通文件") == 0) {
        ret = unlink(real_path);
    } else if (strcmp(type, "目录") == 0) {
        ret = rmdir(real_path); // 注意目录需为空，否则失败
    } else {
        fprintf(stderr, "未知类型: %s\n", type);
        CmdType status = COMMAND_ERROR;
        send(ptask->accept_fd, &status, sizeof(status), 0);
        return;
    }

    if (ret == -1) {
        perror("删除失败");
        CmdType status = COMMAND_ERROR;
        send(ptask->accept_fd, &status, sizeof(status), 0);
        return;
    }

    // 删除数据库记录
    int owner_id = get_owner_id(user->username, ptask->conn);
    int parent_id = get_parent_id(virtual_path, ptask->conn); // 可以优化为直接取 user->current_path
    char* name = strrchr(virtual_path, '/');
    if (name) {
        name++;
    } else {
        name = virtual_path;
    }

    char sql_del[512] = {0};
    snprintf(sql_del, sizeof(sql_del),
             "DELETE FROM file_table WHERE filename='%s' AND owner_id=%d AND parent_id=%d;",
             name, owner_id, parent_id);

    if (mysql_query(ptask->conn, sql_del) != 0) {
        fprintf(stderr, "删除数据库元信息失败: %s\n", mysql_error(ptask->conn));
        CmdType status = COMMAND_ERROR;
        send(ptask->accept_fd, &status, sizeof(status), 0);
        return;
    }

    CmdType status = COMMAND_OK;
    send(ptask->accept_fd, &status, sizeof(status), 0);
    printf("%s '%s' 删除成功，路径: %s\n", type, name, real_path);
}



    void pwdCommand(task_t* ptask){
        session_t* user= session_user(ptask->accept_fd);

       if(user==NULL){
           fprintf(stderr,"session failed\n");
           CmdType status=COMMAND_ERROR;
           send(ptask->accept_fd,&status,sizeof(status),0);
           return;
       }
       
       CmdType status =COMMAND_OK;
        send(ptask->accept_fd, &status, sizeof(status), 0);

        int len=strlen(user->current_path)+1; 
        send(ptask->accept_fd,&len,sizeof(len),0);
        send(ptask->accept_fd,user->current_path,len,0);

    }

    void getsCommand(task_t* ptask){
        session_t* user = session_user(ptask->accept_fd);
      
        if (user == NULL) {
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        char virtual_path[512] = {0};
       if (ptask->data[0] == '/') {
           snprintf(virtual_path, sizeof(virtual_path), "%s", ptask->data);
       } else {
           snprintf(virtual_path, sizeof(virtual_path), "%s/%s", user->current_path, ptask->data);
       }
  


        char real_path[1024] = {0};
        snprintf(real_path, sizeof(real_path), "/home/cccbiji/linux-/server/user_people%s", virtual_path);

         int file_fd = open(real_path, O_RDONLY);
        CmdType status;
        if (file_fd < 0) {
            perror("open file");
            status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }
        status = COMMAND_OK;
        send(ptask->accept_fd, &status, sizeof(status), 0);

        // 发送文件名长度和内容
        int file_name_length = strlen(ptask->data) + 1;
        send(ptask->accept_fd, &file_name_length, sizeof(file_name_length), 0);
        send(ptask->accept_fd, ptask->data, file_name_length, 0);
        // 发送文件长度
        struct stat file_stat;
        fstat(file_fd, &file_stat);
        int file_length = file_stat.st_size;
        send(ptask->accept_fd, &file_length, sizeof(file_length), 0);

        // 根据文件大小选择传输方式
        if (file_length > 100 * 1024 * 1024) {
            getsbigfile(file_fd, ptask->accept_fd, file_length);
        } else {
            getsmallfile(file_fd, ptask->accept_fd, file_length);
        }

    }

    void getsmallfile(int fd,int sockfd,int file_length){
        int offset = 0;
        int ret = recv(sockfd, &offset, sizeof(offset), 0); // 客户端断点续传位置
        if (ret <= 0) {
            perror("recv offset");
            close(fd);
            return;
        }

        if (offset > 0) {
            lseek(fd, offset, SEEK_SET);
        }

        char file_buff[BUFFER_SIZE];
        ssize_t bytes_read;
        int sum = offset;

        while ((bytes_read = read(fd, file_buff, sizeof(file_buff))) > 0) {
            if (send(sockfd, file_buff, bytes_read, 0) < 0) {
                perror("send failed");
                break;
            }
            sum += bytes_read;

            float percent = (float)sum / file_length * 100;
            printf("\r发送文件进度: %.1f%%", percent);
            fflush(stdout);
        }

        printf("\n");

        if (sum != file_length) {
            fprintf(stderr, "文件发送不完整：已发送 %d / %d 字节\n", sum, file_length);
        }

        close(fd);
     }

    void getsbigfile(int fd,int sockfd,int file_length){
        int f_length;

        if (recv(sockfd, &f_length, sizeof(f_length), 0) <= 0) {
            perror("recv failed for resume offset");
            return;
        }

        off_t offset = (off_t)f_length;
        int remaining = file_length - f_length;

        while (remaining > 0) {
            ssize_t sent = sendfile(sockfd, fd, &offset, remaining);
            if (sent == -1) {
                perror("sendfile failed");
                return;
            }
            remaining -= sent;
        }
        printf("大文件已全部传输，文件总大小: %d 字节\n", file_length);
        close(fd);
    }


    void putsCommand(task_t* ptask){
        session_t* user = session_user(ptask->accept_fd);
        if (user == NULL) {
            CmdType status = COMMAND_ERROR;
            send(ptask->accept_fd, &status, sizeof(status), 0);
            return;
        }

        char virtual_path[512] = {0};
        snprintf(virtual_path, sizeof(virtual_path), "%s/%s", user->current_path, ptask->data);

        char real_path[1024] = {0};
        snprintf(real_path, sizeof(real_path), "/home/cccbiji/linux-/server/user_people%s", virtual_path);

        CmdType status;

        // 先接收客户端的准备状态
        if (recv(ptask->accept_fd, &status, sizeof(status), 0) <= 0) {
            perror("recv status failed");
            return;
        }

        if (status == COMMAND_ERROR) {
            printf("客户端出现问题，上传文件失败\n");
            return;
        } else if (status == COMMAND_OK) {
            int file_name_length = 0;
            if (recv(ptask->accept_fd, &file_name_length, sizeof(file_name_length), 0) <= 0) {
                perror("recv file_name_length failed");
                return;
            }

            char filename[100] = {0};
            if (recv(ptask->accept_fd, filename, file_name_length, 0) <= 0) {
                perror("recv filename failed");
                return;
            }

            // **新增：接收 SHA1 值（固定长度40字节）**
            char sha1[SHA1_STR_LEN] = {0};
            if (recv(ptask->accept_fd, sha1, SHA1_STR_LEN - 1, 0) <= 0) {
                perror("recv sha1 failed");
                return;
            }

            // 查询数据库是否已有该文件
            int file_exists = check_file_exists(sha1, ptask);

            int file_length = 0;
            if (!file_exists) {
                // 不存在，继续接收文件大小和文件内容
                if (recv(ptask->accept_fd, &file_length, sizeof(file_length), 0) <= 0) {
                    perror("recv file_length failed");
                    return;
                }

                // 根据文件大小选择接收函数
                if (file_length > 100 * 1024 * 1024) { // >100MB
                    putsbig_recv(real_path, user, file_length);
                } else {
                    putsmall_recv(real_path, user, file_length);
                }

                insert_file_metadata(ptask,virtual_path,real_path,sha1,filename,file_length);
        
            } else {
                // 秒传文件，文件长度从数据库获取（可选）
                printf("文件秒传，跳过上传\n");
            insert_file_metadata_speed(ptask,filename,virtual_path,sha1 );

        }
    }
    }


    int check_file_exists(const char* sha1, task_t* ptask){
        char sql[256] = {0};
        snprintf(sql, sizeof(sql), "SELECT COUNT(*) FROM server_file WHERE sha1='%s';", sha1);

        if (mysql_query(ptask->conn, sql) != 0) {
            fprintf(stderr, "check_file_exists 查询失败: %s\n", mysql_error(ptask->conn));
            return 0;  // 失败视为文件不存在
        }

        MYSQL_RES* result = mysql_store_result(ptask->conn);
        if (result == NULL) {
            fprintf(stderr, "check_file_exists 获取结果失败: %s\n", mysql_error(ptask->conn));
            return 0;
        }

        MYSQL_ROW row = mysql_fetch_row(result);
        int count = 0;
        if (row != NULL) {
            count = atoi(row[0]);
        }
        send(ptask->accept_fd,&count,sizeof(count),0);

        mysql_free_result(result);
        return (count > 0) ? 1 : 0;
    }

void insert_file_metadata_speed(task_t* ptask, char* filename, char* virtual_path, char* sha1) {
    if (!ptask || !filename || !virtual_path || !sha1) {
        fprintf(stderr, "insert_file_metadata_speed: 参数无效\n");
        return;
    }

    session_t* user = session_user(ptask->accept_fd);
    if (!user) {
        fprintf(stderr, "insert_file_metadata_speed: 获取用户信息失败\n");
        return;
    }

    MYSQL* conn = ptask->conn;
    if (!conn) {
        fprintf(stderr, "insert_file_metadata_speed: 数据库连接无效\n");
        return;
    }

    // 查询 server_file 表，获取 filesize 和 type
    char sql_select[512];
    snprintf(sql_select, sizeof(sql_select),
             "SELECT size, type FROM server_file WHERE sha1='%s' LIMIT 1;",
             sha1);

    if (mysql_query(conn, sql_select) != 0) {
        fprintf(stderr, "insert_file_metadata_speed: 查询server_file失败: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        fprintf(stderr, "insert_file_metadata_speed: 获取查询结果失败: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        fprintf(stderr, "insert_file_metadata_speed: server_file中无匹配sha1文件\n");
        mysql_free_result(res);
        return;
    }

    // 读取 filesize 和 type，注意row[0], row[1]可能为NULL，需判断
    int filesize = 0;
    if (row[0] != NULL) {
        filesize = atoi(row[0]);
    }

    char type_str[64] = {0};
    if (row[1] != NULL) {
        strncpy(type_str, row[1], sizeof(type_str) - 1);
        type_str[sizeof(type_str) - 1] = '\0';
    }

    mysql_free_result(res);

    // 获取父目录ID
    int parent_id = get_parent_id(virtual_path, conn);
    if (parent_id < 0) {
        fprintf(stderr, "insert_file_metadata_speed: 获取父目录ID失败\n");
        return;
    }

    // 获取用户ID
    int owner_id = get_owner_id(user->username, conn);
    if (owner_id < 0) {
        fprintf(stderr, "insert_file_metadata_speed: 获取用户ID失败\n");
        return;
    }

    // 构造插入语句
    char sql_insert[1024];
    snprintf(sql_insert, sizeof(sql_insert),
             "INSERT INTO file_table(parent_id, filename, owner_id, sha1, filesize, type) "
             "VALUES(%d, '%s', %d, '%s', %d, '%s');",
             parent_id, filename, owner_id, sha1, filesize, type_str);

    if (mysql_query(conn, sql_insert) != 0) {
        fprintf(stderr, "insert_file_metadata_speed: 插入file_table失败: %s\n", mysql_error(conn));
    } else {
        printf("文件 %s 已通过秒传添加至用户文件表\n", filename);
    }
}




    void insert_file_metadata(task_t* ptask,char* virtual_path,  char* full_path,const char* sha1, const char* filename,int file_length) {


        // 3. 获取文件类型（返回字符串，如 "目录"、"文件"）
        const char* type_str = get_file_type(full_path);

        int parent_id = get_parent_id(virtual_path, ptask->conn);

        // 6. 获取 owner_id
        session_t* user = session_user(ptask->accept_fd);
        int owner_id = get_owner_id(user->username, ptask->conn);

        // 7. 插入 server_file（如果不存在）
        char sql[1024];
        snprintf(sql, sizeof(sql),
                 "INSERT IGNORE INTO server_file(sha1, size,type) VALUES('%s', %d,'%s');",
                 sha1, file_length,type_str);
        if (mysql_query(ptask->conn, sql) != 0) {
            fprintf(stderr, "插入 server_file 失败: %s\n", mysql_error(ptask->conn));
            return;
        }

        // 8. 插入 file_table
         snprintf(sql, sizeof(sql),
             "INSERT INTO file_table(parent_id, filename, owner_id, sha1, filesize, type) "
             "VALUES(%d, '%s', %d, '%s', %d, '%s');",
             parent_id, filename, owner_id, sha1, file_length, type_str);

        if (mysql_query(ptask->conn, sql) != 0) {
            fprintf(stderr, "插入 file_table 失败: %s\n", mysql_error(ptask->conn));
            return;
        }

        printf("文件 %s 元信息已成功插入数据库。\n", filename);
    }


    void putsmall_recv(char* filename, session_t* user, int file_length) {
        int file_fd = open(filename, O_RDWR | O_CREAT, 0755);
        if (file_fd < 0) {
            perror("open");
            return;
        }

        struct stat file_buff;
        if (fstat(file_fd, &file_buff) < 0) {
            perror("fstat");
            close(file_fd);
            return;
        }

        int f_length = file_buff.st_size;
        // 发送已接收文件大小给客户端，实现断点续传
        if (send(user->sockfd, &f_length, sizeof(f_length), 0) < 0) {
            perror("send");
            close(file_fd);
            return;
        }

        char data[BUFFER_SIZE];
        int sum = 0;
        int r;
        float percent = 0.0f;

        if (f_length == 0) {
            // 新文件，预分配文件大小
            if (ftruncate(file_fd, file_length) < 0) {
                perror("ftruncate");
                close(file_fd);
                return;
            }
        } else {
        // 续传，文件指针移动到当前大小位置
        sum = f_length;
        if (lseek(file_fd, f_length, SEEK_SET) < 0) {
            perror("lseek");
            close(file_fd);
            return;
        }
    }

    while (sum < file_length) {
        r = recv(user->sockfd, data, sizeof(data), 0);
        if (r <= 0) {
            perror("recv");
            break;
        }

        ssize_t bytes_written = write(file_fd, data, r);
        if (bytes_written != r) {
            perror("write");
            break;
        }

        sum += bytes_written;
        percent = (float)sum / file_length * 100;
        printf("\r接收文件进度: %.1f%%", percent);
        fflush(stdout);
    }

    if (sum == file_length) {
        printf("\n文件接收完成！\n");
    } else {
        fprintf(stderr, "\n文件接收未完成，期望: %d, 实际: %d\n", file_length, sum);
    }

    close(file_fd);
}


void putsbig_recv(char* filename, session_t* user, int file_length) {
    int file_fd = open(filename, O_RDWR | O_CREAT, 0775);
    if (file_fd < 0) {
        perror("open");
        return;
    }

    struct stat file_buff;
    if (fstat(file_fd, &file_buff) < 0) {
        perror("fstat");
        close(file_fd);
        return;
    }

    int f_length = file_buff.st_size;

    // 发送已接收的文件大小，支持断点续传
    if (send(user->sockfd, &f_length, sizeof(f_length), 0) < 0) {
        perror("send");
        close(file_fd);
        return;
    }

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        close(file_fd);
        return;
    }

    int n = file_length - f_length;  // 还需接收的字节数
    off_t offset = f_length;

    while (n > 0) {
        int splice_move = n > 65536 ? 65536 : n;

        // 从 socket 通过管道写入内核缓冲
        int r = splice(user->sockfd, NULL, pipefd[1], NULL, splice_move, SPLICE_F_MORE);
        if (r == -1) {
            perror("splice sockfd->pipe");
            break;
        }
        if (r == 0) { // 对端关闭连接
            break;
        }

        int to_write = r;

        // 从管道写入文件，使用偏移量保证文件写入位置正确
        while (to_write > 0) {
            int w = splice(pipefd[0], NULL, file_fd, &offset, to_write, SPLICE_F_MORE);
            if (w == -1) {
                perror("splice pipe->file");
                break;
            }
            to_write -= w;
        }

        if (to_write > 0) {
            // 写入不完整，跳出循环
            break;
        }

        n -= r;
    }

    close(pipefd[0]);
    close(pipefd[1]);
    close(file_fd);

    if (n == 0) {
        printf("文件全部接收\n");
    } else {
        printf("文件传输未完成，剩余 %d 字节\n", n);
    }
} 

void DelEpollfd(int epfd,int fd){
    if(epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL)==-1){
        perror("epoll DEL failed ");
 }
}


