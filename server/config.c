#include"config.h"
#include "hashtable.h"
void readConfig(const char* filename,HashTable* ht){
    FILE* fp=fopen(filename,"r");
    if(fp==NULL){
        printf("open file %s error.\n",filename);
        return;
    }
    char buff[128]={0};
    
    while(fgets(buff,sizeof(buff),fp)!=NULL){
        char* strs[3]={0};
        int cnt=0;
        splitString(buff,"=",strs,3,&cnt);    
        char* value=(char*)calloc(1,strlen(strs[1])+1);
        strcpy(value,strs[1]);
        insert(ht,strs[0],value);
        freeStrs(strs,cnt);
        }
    fclose(fp);
 }


void splitString(char* str,const char* delim,char** outStrs,int maxToken,int* tokenCount){
  *tokenCount=0;
  size_t len= strlen(str);
  if(len>0&&(str[len-1]=='\n'|| str[len-1]=='\r')){
      str[len-1]='\0';
  }
      len=strlen(str);
      char* token=strtok(str,delim);
      while(token!= NULL&& *tokenCount<maxToken){
          outStrs[*tokenCount]=strdup(token);
          if(!outStrs[*tokenCount]){
              for(int i=0;i<*tokenCount;i++){
                  free(outStrs);
                  outStrs[i]=NULL;
              }
          *tokenCount=0;
          return;
          }
          (*tokenCount)++;
          token=strtok(NULL,delim);
      }
}


void freeStrs(char** strs,int cnt){
    for(int i=0 ; i<cnt; i++)
        free(strs[i]);
    
}
