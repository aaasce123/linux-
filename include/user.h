#ifndef USER_H
#define USER_H
#include<shadow.h>
#include"ser_main.h"
#include"session.h"

char* Rand_salt();
void user_Register1(task_t* t);
void user_Register2(task_t* t);
void user_Login1(task_t* t);
void user_Login2(task_t* t);
void get_setting(char* setting,char* str);

#endif
