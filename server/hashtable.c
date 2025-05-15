#include"hashtable.h"

unsigned int hash(const char* key){
    unsigned int hash=0;
    while(*key){
        hash=hash*131+(unsigned char)(*key);
        key++;
    }
    return hash % MAX_SIZE;
}

void initHashTable(HashTable* ht){
    if(ht==NULL)
        return;
    ht->size=0;
    memset(ht,0,sizeof(*ht));
}

void insert(HashTable* ht,const char* key,void* value){
    if(ht==NULL||key==NULL)
        return;
    if(ht->size>=MAX_SIZE){
        fprintf(stderr,"ht size error\n");
        return;
    }
    unsigned int index=hash(key);
    unsigned int original_index= index;
    int i=0;
//开放定址法
    while(ht->table[index].key[0]!= '\0'){
        //如果已经判断是否一样，一样就更新，不一样就移动
        if(strcmp(ht->table[index].key,key)==0){
         ht->table[index].value =value;
         return;
        }
        i++;
        index =(original_index+ i)%MAX_SIZE;
    }
    //插入
    strncpy(ht->table[index].key,key,sizeof(ht->table[index].key)-1);
    ht->table[index].key[sizeof(ht->table[index].key)-1]='\0';
    ht->table[index].value=value;
    ht->size++;
}

void* find(HashTable*ht,const char* key){
   unsigned int index=hash(key);
   unsigned int original_index=index;
   int i=0;
   
   while(ht->table[index].key[0]!='\0'){
       if(strcmp(ht->table[index].key,key)==0){
           return ht->table[index].value;
       }
       i++;
       index=(original_index+i)%MAX_SIZE;
       if(index==original_index){
           break;
       }
   }
   return NULL;
}

void erase(HashTable* ht,const char* key){
    
   unsigned int index=hash(key);
   unsigned int original_index=index;
   int i=0;

   while(ht->table[index].key[0]!='\0'){
       if(strcmp(ht->table[index].key,key)==0){
           memset(&ht->table[index],0,sizeof(ht->table[index]));
           ht->size--;
           return;
       }
       i++;
       index=(original_index+i)%MAX_SIZE;
       if(index==original_index){
           return;
       }
   }
}

void printHashTable(HashTable* ht){
    for(int i=0;i<MAX_SIZE;i++){
        if(ht->table[i].key[0]!='\0')
            printf("key:%s value:%s\n",ht->table[i].key,(char *)ht->table[i].value);
    }
}
void destroyHashTable(HashTable* ht){
    for(int i=0;i<MAX_SIZE;i++){
        if(ht->table[i].key[0]!='\0')
            free(ht->table[i].value);
    }
}


