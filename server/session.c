#include "session.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include<string.h>
#include<stdio.h>

static session_t session_list[MAX_SESSIONS];
static pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

void session_init(){
    pthread_mutex_lock(&mutex);
    memset(session_list,0,sizeof(session_list));
    pthread_mutex_unlock(&mutex);
}

int session_add(int sockfd,char* username,char* init_path){
    if(!username||!init_path)
        return -1;
    pthread_mutex_lock(&mutex);
    for(int i=0;i<MAX_SESSIONS;++i){
        if(session_list[i].sockfd==0){
            session_list[i].sockfd=sockfd;
            strncpy(session_list[i].username,username,USERNAME_MAX_LEN-1);
            strncpy(session_list[i].current_path,init_path,PATHN_MAX_LEN-1);
            pthread_mutex_unlock(&mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&mutex);
    return -1;
    
}

session_t* session_user(int sockfd){
    pthread_mutex_lock(&mutex);
    for(int i=0;i<MAX_SESSIONS;i++){
        if(session_list[i].sockfd==sockfd){
            pthread_mutex_unlock(&mutex);
            return &session_list[i];
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}

session_t* session_user_by_name(const char* username) {
    if (username == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (strcmp(session_list[i].username, username) == 0) {
            pthread_mutex_unlock(&mutex);
            return &session_list[i];
        }
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}


int session_set_path(int sockfd,char* new_path){
    if(!new_path) return -1;

    pthread_mutex_lock(&mutex);

    for(int i=0;i<MAX_SESSIONS;i++){
        if(session_list[i].sockfd==sockfd){
        strncpy(session_list[i].current_path,new_path,PATHN_MAX_LEN-1);
        pthread_mutex_unlock(&mutex);
        return 0;
        }
    }
    pthread_mutex_unlock(&mutex);
    return -1;

}

void session_remove(int sockfd){
    pthread_mutex_lock(&mutex);
    for(int i=0;i<MAX_SESSIONS;i++){
       if(session_list[i].sockfd==sockfd){
           memset(&session_list[i],0,sizeof(session_t));
           break;
       }
    }
    pthread_mutex_unlock(&mutex);
}




