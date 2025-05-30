#ifndef SHA1_H
#define SHA1_H

#define SHA1_STR_LEN 41
#define _POSIX_C_SOURCE 200809L
#include"sql.h"
#include<stdio.h>
#include<openssl/sha.h>
#include<sys/stat.h>

void file_to_sha1(const char* filename,char sha1_str[SHA1_STR_LEN]);

int get_file_size(const char* filename);

const char* get_file_type(const char* filename);
int get_parent_id( char* path,MYSQL* conn);
int get_dir_id(char* path,MYSQL* conn);
int get_owner_id(char* username,MYSQL* conn);

void normalize_path(char* path);

#endif

