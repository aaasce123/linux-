#ifndef CONFIG_H
#define CONFIG_H
#define _POSIX_C_SOURCE 200809L
#include<string.h>
#include<stdlib.h>
#include<malloc.h>
#include "hashtable.h"
#define IP "ip"
#define PORT "port"
#define THREAD_NUM "thread_num"

void readConfig(const char* filename,HashTable* ht);

void splitString(char* str,const char* delim,char **outStrs,int maxToken,int* tokenCount);
void freeStrs(char** strs,int cnt);
#endif
