#ifndef HASHTABLE_H
#define HASHTABLE_H
#include<stdio.h>
#include<malloc.h>
#include<string.h>
#define MAX_SIZE 100
#define EMPTY NULL

typedef struct{
    char key[50];
    void * value;
}KeyValue;

typedef struct{
    KeyValue table[MAX_SIZE];
    int size;
}HashTable;

//hash函数
unsigned int hash(const char* key);


//初始化哈希表
void initHashTable(HashTable* ht);

//插入
void insert(HashTable* ht,const char* key,void* value);

//查找
void* find(HashTable* ht, const char* key);

//删除键值
void erase(HashTable* ht,const char* key);

void printHashTable(HashTable* ht);
//销毁
void destroyHashTable(HashTable* ht);



#endif
