//
// Created by apple on 6/1/2023.
//


#ifndef LOG_H_
#define LOG_H_

#include<stdio.h>
enum MyLogLevel
{
    ERROR_LEVEL = 1,
    WARN_LEVEL  = 2,
    INFO_LEVEL  = 3,
    DEBUG_LEVEL = 4,
};

void mylog1(const char* filename, int line, enum MyLogLevel level, const char* fmt, ...);



#define LOGE printf
#define LOGW printf
#define LOGI printf
#define LOGE printf


//#define LOGE(format,...) mylog1(__FILE__, __LINE__,ERROR_LEVEL,format,## __VA_ARGS__)
//#define LOGI(format,...) mylog1(__FILE__, __LINE__,INFO_LEVEL,format,## __VA_ARGS__)
//#define LOGD(format,...) mylog1(__FILE__, __LINE__,DEBUG_LEVEL,format,## __VA_ARGS__)
//#define LOGW(format,...) mylog1(__FILE__, __LINE__,WARN_LEVEL,format,## __VA_ARGS__)



#endif
