#include"log.h"
#include <sys/syslog.h>

void log_init(const char* ident,int option, int facility){

    openlog(ident,option,facility);
}

void log_close(){
    closelog();
}

void log_write(LogLevel level, const char* file, int line,const char* fun, const char* format){
}
//日志文件;
