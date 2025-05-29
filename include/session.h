#ifndef SESSION_H   
#define SESSION_H

#include<pthread.h>

#define MAX_SESSIONS 1024
#define USERNAME_MAX_LEN 64
#define PATHN_MAX_LEN 256

typedef struct{
    int sockfd;
    char username[USERNAME_MAX_LEN];
    char current_path[PATHN_MAX_LEN];
}session_t;

void session_init();

int session_add(int sockfd,char* username,char* init_path);

session_t* session_user(int sockfd);

int session_set_path(int sockfd, char* new_path);

void session_remove(int sockfd);

#endif
