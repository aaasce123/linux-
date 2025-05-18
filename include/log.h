#ifndef LOG_H
#define LOG_H
#include<syslog.h>

typedef enum{
    DEBUG = 7,  // 对应 syslog LOG_DEBUG
    INFO  = 6,  // 对应 syslog LOG_INFO
    WARN  = 4,  // 对应 syslog LOG_WARNING
    ERROR = 3,  // 对应 syslog LOG_ERR
    FATAL = 2   // 对应 syslog LOG_CRIT
}LogLevel;

void log_init(const char* ident, int option,int facility);

void log_close();

void log_write(LogLevel level,const char* file, int line, const char* fun,const char* format);

#endif


