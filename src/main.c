#include"main.h"
#include <stdint.h>
#include <sys/epoll.h>
#include<unistd.h>
void check_argc(int num){

    if(num !=3){
        fprintf(stderr,"argc error");
        exit(EXIT_FAILURE);
    } 
}

void addEpollfd(int epfd, int fd, uint32_t events){
    struct epoll_event ev;
    ev.events=events;
    ev.data.fd=fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);   
    error(ret,-1,"epoll_ctl failed");
}

void dotask(){
    printf("12\n");
  sleep(3);
}


 
