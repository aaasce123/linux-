#include"user.h"
#include "ser_main.h"
#include <mysql/mysql.h>
#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include "user.h"
#include<mysql/mysql.h>
#include <sys/socket.h>
#include"sql.h"

char* Rand_salt(){

 const char charset[] = "./0123456789ABCDEFGHIJKLMNOPQR"
                        "STUVWXYZabcdefghijklmnopqrstuvwxyz";
 char* salt=(char*)calloc(21,sizeof(char));
salt[0]='$';
salt[1]='6';
salt[2]='$';
srand(time(NULL));
 for(int i=3;i<19;i++){
      salt[i]=charset[rand()%64];
 }
 salt[19]='$';
 salt[20]='\0';
 return salt;
}
void user_Register1(task_t* t){
    MYSQL* conn =t->conn;
    char query[256];
   snprintf(query,sizeof(query),"select id from user where username='%s'",t->data);
   if(mysql_query(conn,query)){
       fprintf(stderr,"Register1 failed:%s\n",mysql_error(conn));
       return;
   }
   MYSQL_RES* result=mysql_store_result(conn);
   if(result==NULL){
       fprintf(stderr,"Register1 result failed: %s\n",mysql_error(conn));
       return;
   }
   int num_rows=mysql_num_rows(result);
   if(num_rows>0){
       CmdType status=TASK_REGISTER1_RESP_ERROR;
       send(t->accept_fd,&status,sizeof(status),0);
   }
   else{
       CmdType status=TASK_REGISTER1_RESP_OK;
       send(t->accept_fd,&status,sizeof(status),0);

       char* salt=Rand_salt();
       int len=strlen(salt)+1;
       send(t->accept_fd,&len,sizeof(len),0);
       send(t->accept_fd,salt,len,0);
    
   }
mysql_free_result(result);
}
void user_Register2(task_t* t){
    MYSQL* conn =t->conn;
    char* username=t->data;
    char* passwd=t->data+strlen(username)+1;
    char* cryptpassed=t->data+strlen(username)+1+strlen(passwd)+1;

    char query[256];
   snprintf(query,sizeof(query),"Insert into user (username,cryptpasswd,pwd,passwd)"
                                  "values('%s','%s','../','%s')",username,cryptpassed,passwd);                                 
    if(mysql_query(conn,query)){
        CmdType status=TASK_REGISTER2_RESP_ERROR;
        send(t->accept_fd,&status,sizeof(status),0);
       fprintf(stderr,"Register2 insert2 :%s\n",mysql_error(conn));
        return;
       }
    else{
       CmdType status=TASK_REGISTER2_RESP_OK;
       send(t->accept_fd,&status,sizeof(status),0);
       }
}
void user_Login1(task_t* t){
       MYSQL* conn =t->conn;                                                   
       char query[256];
       snprintf(query,sizeof(query),"select * from user where username='%s'",t->data);
       if(mysql_query(conn,query)){
           fprintf(stderr,"Login1 failed:%s\n",mysql_error(conn));
           return;
       }
       MYSQL_RES* result=mysql_store_result(conn);
       if(result==NULL){
           fprintf(stderr,"Login1 result failed: %s\n",mysql_error(conn));
           return;
       }
       int num_rows=mysql_num_rows(result);
       if(num_rows==0){
           CmdType status=TASK_LOGIN_SECTION1_RESP_ERROR;
           send(t->accept_fd,&status,sizeof(status),0);
       }else{
          CmdType status=TASK_LOGIN_SECTION1_RESP_OK;
          send(t->accept_fd,&status,sizeof(status),0);
          MYSQL_ROW row; 
          row=mysql_fetch_row(result);
          char salt[25];
          get_setting(salt,row[2]);
          int len=strlen(salt)+1;
          send(t->accept_fd,&len,sizeof(len),0);
          send(t->accept_fd,salt,len,0);
       }
mysql_free_result(result);
}
void user_Login2(task_t* t){
           MYSQL* conn =t->conn;
    char* username=t->data;
    char* cryptpassed=t->data+strlen(username)+1;
           char query[256];
           snprintf(query,sizeof(query),"select * from user where username='%s'",username);
           if(mysql_query(conn,query)){
               fprintf(stderr,"Login2 failed:%s\n",mysql_error(conn));
               return;
           }
           MYSQL_RES* result=mysql_store_result(conn);
           if(result==NULL){
               fprintf(stderr,"Login2 result failed: %s\n",mysql_error(conn));
               return;
           }
           MYSQL_ROW row;
           row=mysql_fetch_row(result);
           if(strcmp(cryptpassed,row[2])==0){
            CmdType status=TASK_LOGIN_SECTION2_RESP_OK;
            send(t->accept_fd,&status,sizeof(status),0);
            
            int len=strlen(row[3]);
            send(t->accept_fd,&len,sizeof(len),0);
            send(t->accept_fd,row[3],len,0);
           }else{
               CmdType status=TASK_LOGIN_SECTION2_RESP_ERROR;
               send(t->accept_fd,&status,sizeof(status),0);
           }
mysql_free_result(result);
}



void get_setting(char* setting, char* str){
  int i,j;
  for(i=0,j=0;str[i]&&j!=4;i++){
      if(str[i]=='$'){
          j++;
      }
  }
  strncpy(setting,str,i);
  setting[i]='\0';
  }
