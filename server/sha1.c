#include"sha1.h"
#include <cstdio>
#include <openssl/sha.h>
#include <stddef.h>
#include <stdio.h>
void file_to_sha1(const char* filename,char sha1_str[SHA1_STR_LEN]){
   FILE * file=fopen(filename,"rb");
   if(file ==NULL){
       perror("file_to_sha1 fopen failed");
       sha1_str[0]='\0';
       return;
   }
   SHA_CTX ctx;
   SHA1_Init(&ctx); 

   char buff[4096];
   size_t bytes_read;

   while((bytes_read=fread(buff,1,sizeof(buff),file))>0){
     if(SHA1_Update(&ctx,buff,bytes_read)!=1){
         fprintf(stderr,"SHA1_Update failed\n");
         fclose(file);
         sha1_str[0]='\0';
         return;
         }
     }

   //检验是否出错
   if(ferror(file)){
       perror("fread failed");
       fclose(file);
       sha1_str[0]='\0';
       return;
   }
   fclose(file);
   
   char sha1_digest[SHA_DIGEST_LENGTH];
   if(SHA1_Final(sha1_digest,&ctx)!=1){
       fprintf(stderr,"SHA1_Final failed\n");
       sha1_str[0]='\0';
       return;
   }
   for(int i=0;i<SHA_DIGEST_LENGTH;i++){
       sprintf(sha1_str+i*2,"%02x",sha1_digest[i]);
   }
   sha1_str[SHA1_STR_LEN-1]='\0';
}

int get_file_size(const char* filename){
    struct stat st;
    if(stat(filename,&st)==0){
        return (int)st.st_size;
    }
    return -1;
}

const char* get_file_type(const char* filename){
     struct stat st;

    if (stat(filename, &st) != 0) {
        return "未知类型";  // 获取失败
    }
if (S_ISREG(st.st_mode))  return "普通文件";
if (S_ISDIR(st.st_mode))  return "目录";
if (S_ISLNK(st.st_mode))  return "符号链接";
if (S_ISCHR(st.st_mode))  return "字符设备";
if (S_ISBLK(st.st_mode))  return "块设备";
if (S_ISFIFO(st.st_mode)) return "管道文件";
if (S_ISSOCK(st.st_mode)) return "套接字";
return "未知类型";

}
