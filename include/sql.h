#ifndef SQL_H
#define SQL_H
#include<mysql/mysql.h>

typedef struct{
 char* host;
 int  port;
 char* user;
 char* passwd;
 char* db;
}db_connect;

MYSQL* mysql_db_con();

#endif
