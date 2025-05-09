#include"threadpool.h"
#include <stdint.h>
#include<stdlib.h>
#include<stdio.h>
void dotask();
void check_argc(int num);
void addEpollfd(int epfd, int fd,uint32_t events);

