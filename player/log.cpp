//
// Created by apple on 13/1/2023.
//


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//#include <sys/time.h>

#include "log.h"

//#ifndef LOGLEVEL
//#define LOGLEVEL DEBUG_L
//#endif

// 使用了GNU C扩展语法，只在gcc（C语言）生效，
// g++的c++版本编译不通过
static const char* s_loginfo[] = {
        [ERROR_LEVEL] = "ERROR",
        [WARN_LEVEL]  = "WARN",
        [INFO_LEVEL]  = "INFO",
        [DEBUG_LEVEL] = "DEBUG",
};



void mylog1(const char* filename, int line, enum MyLogLevel level, const char* fmt, ...)
{
    if(level > DEBUG_LEVEL)
        return;

    va_list arg_list;
    char buf[1024];
    memset(buf, 0, 1024);
    va_start(arg_list, fmt);
    vsnprintf(buf, 1024, fmt, arg_list);
    char time[32] = {0};

    // 去掉*可能*存在的目录路径，只保留文件名
    const char* tmp = strrchr(filename, '/');
    if (!tmp) tmp = filename;
    else tmp++;
//    get_timestamp(time);

    switch(level){
        case DEBUG_LEVEL:
            //绿色
            printf("\033[1;32m%s[%s] [%s:%d] %s\n\033[0m", time, s_loginfo[level], tmp, line, buf);
            break;
        case INFO_LEVEL:
            //蓝色
//            printf("\033[1;34m%s[%s] [%s:%d] %s\033[0m", time, s_loginfo[level], tmp, line, buf);
            printf("%s[%s] [%s:%d] %s\n", time, s_loginfo[level], tmp, line, buf);
            break;
        case ERROR_LEVEL:
            //红色
            printf("\033[1;31m%s[%s] [%s:%d] %s\n\033[0m", time, s_loginfo[level], tmp, line, buf);
            break;
        case WARN_LEVEL:
            //黄色
            printf("\033[1;33m%s[%s] [%s:%d] %s\n\033[0m", time, s_loginfo[level], tmp, line, buf);
//            printf("\033[1;33m%s[%s] [%s:%d] %s\n\033[0m", time, s_loginfo[level], tmp, line, buf);
            break;
    }
    va_end(arg_list);
}
