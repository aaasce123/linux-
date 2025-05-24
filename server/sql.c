#include"sql.h"
#include"hashtable.h"
#include"config.h"
#include<malloc.h>
#include <mysql/mysql.h>

MYSQL* mysql_db_con(){

    HashTable* ht=calloc(1,sizeof(HashTable));
    initHashTable(ht);
    readConfig("../config/db.conf",ht);
   db_connect db_con; 
   db_con.host=(char*)find(ht,"host");
   db_con.port=atoi((char*)find(ht,"port"));
   db_con.user=(char*)find(ht,"user");
   db_con.passwd=(char*)find(ht,"password");
   db_con.db=(char*)find(ht,"database");

    MYSQL* conn=mysql_init(NULL);
    if(conn==NULL){
    fprintf(stderr,"musql init error\n");
    return NULL;
    }

  if(mysql_real_connect(conn,db_con.host,db_con.user,
     db_con.passwd,db_con.db,db_con.port,NULL,0)==NULL){

      fprintf(stderr,"mysql_connect error\n");
      destroyHashTable(ht);
      free(ht);
      mysql_close(conn);
      return NULL;
  };
  destroyHashTable(ht);
  free(ht);
  printf("连接成功\n");
  return conn;
} 


