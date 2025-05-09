#include "socket_utils.h"
#include <sys/socket.h>
#include"threadpool.h"

int main(int argc, char *argv[]){ 

    if(argc !=3){
        fprintf(stderr,"argc error");
        exit(EXIT_FAILURE);
    } 

  int client_fd=socket(AF_INET,SOCK_STREAM,0);

   char *ip_addr=argv[1];
   struct sockaddr_in serv_addr;
   memset(&serv_addr ,0,sizeof(serv_addr));
   serv_addr.sin_family=AF_INET;
   inet_pton(AF_INET, ip_addr,&serv_addr.sin_addr);
   serv_addr.sin_port=htons(atoi(argv[2]));

   int ret =connect(client_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
   error(ret,-1,"connect failed ");
   while(1);
}


