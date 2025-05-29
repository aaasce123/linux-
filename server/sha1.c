#include"sha1.h"
#include <openssl/sha.h>
#include <stddef.h>
#include<string.h>
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
   
   unsigned char sha1_digest[SHA_DIGEST_LENGTH];
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

int get_parent_id( char* path, MYSQL* conn) {

    if (path == NULL || conn == NULL) {
        return -1;
    }

    char norm_path[512];
    strncpy(norm_path, path, sizeof(norm_path));
    norm_path[sizeof(norm_path) - 1] = '\0';
    normalize_path(norm_path);

    // 提取上级路径
    char parent_path[512];
    strncpy(parent_path, norm_path, sizeof(parent_path));
    parent_path[sizeof(parent_path) - 1] = '\0';

    char* last_slash = strrchr(parent_path, '/');
    if (last_slash == NULL) {
        return 0;
    }

    *last_slash = '\0';
    if (parent_path[0] == '\0') {
        return 0;  // 根目录
    }

    return get_dir_id(parent_path, conn);
}



int get_owner_id(char* username, MYSQL* conn) {
    if (username == NULL || conn == NULL) {
        return 0;
    }

    char sql[256];
    snprintf(sql, sizeof(sql),
             "SELECT id FROM user WHERE username='%s' LIMIT 1;",
             username);

    if (mysql_query(conn, sql)) {
        fprintf(stderr, "数据库查询失败: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "获取结果失败: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    int owner_id = 0;
    if (row != NULL && row[0] != NULL) {
        owner_id = atoi(row[0]);
    }

    mysql_free_result(result);
    return owner_id;
}

 void normalize_path(char* path) {
     char* dst = path;
     char* src = path;
     while (*src) {
         *dst++ = *src;
         if (*src == '/') {
             while (*(src + 1) == '/') {
                 src++;
             }
         }
         src++;
     }
     *dst = '\0';
 }

int get_dir_id( char* path, MYSQL* conn) {
    if (path == NULL || conn == NULL || strlen(path) == 0) {
        return 0;  // 根目录
    }

    char norm_path[512];
    strncpy(norm_path, path, sizeof(norm_path));
    norm_path[sizeof(norm_path) - 1] = '\0';
    normalize_path(norm_path);

    char parent_path[512];
    strncpy(parent_path, norm_path, sizeof(parent_path));
    parent_path[sizeof(parent_path) - 1] = '\0';

    char* last_slash = strrchr(parent_path, '/');
    char* dirname = NULL;

    if (last_slash != NULL) {
        dirname = last_slash + 1;
        *last_slash = '\0'; // parent_path = 上级路径
    } else {
        dirname = parent_path;
        parent_path[0] = '\0'; // 上级是根目录
    }

    int parent_id = 0;
    if (parent_path[0] != '\0') {
        parent_id = get_dir_id(parent_path, conn);
        if (parent_id < 0) {
            return -1;
        }
    }

    // 查找目录
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id FROM file_table WHERE filename='%s' AND parent_id=%d AND type='目录' LIMIT 1;",
             dirname, parent_id);

    if (mysql_query(conn, sql) != 0) {
        return -1;
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (res == NULL) {
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    int dir_id = -1;
    if (row && row[0]) {
        dir_id = atoi(row[0]);
    }

    mysql_free_result(res);
    return dir_id;
}

