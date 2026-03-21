/**
 * @brief 简单日志类
 * @date 2026.03.19
 */
#pragma once

#include <string>
#include "noncopyable.h"
#include <iostream>

#define MUDEBUG

#define LOG_INFO(logmsgFormat, ...) do{                     \
        Logger& logger = Logger::Instance();                \
        logger.setLogLevel(INFO);                           \
        char buf[1024] = { 0 };                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)

#define LOG_ERROR(logmsgFormat, ...) do{                    \
        Logger& logger = Logger::Instance();                \
        logger.setLogLevel(ERROR);                          \
        char buf[1024] = { 0 };                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)

#define LOG_FATAL(logmsgFormat, ...) do{                    \
        Logger& logger = Logger::Instance();                \
        logger.setLogLevel(FATAL);                          \
        char buf[1024] = { 0 };                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
        exit(-1);                                           \
    }while(0)

#ifdef MUDEBUG
#define LOG_DEBUG(logmsgFormat, ...) do{                    \
        Logger& logger = Logger::Instance();                \
        logger.setLogLevel(DEBUG);                          \
        char buf[1024] = { 0 };                             \
        snprintf(buf, 1024, logmsgFormat, ##__VA_ARGS__);   \
        logger.log(buf);                                    \
    }while(0)
#else
#define LOG_DEBUG(logmsgFormat, ...)
#endif


// 日志级别枚举
enum LogLevel {
    INFO,   // 普通信息
    ERROR,  // 错误信息
    FATAL,  // core信息
    DEBUG,  // 调试信息
};

using namespace muduo;

class Logger : noncopyable {
public:
    static Logger& Instance();
    void setLogLevel(int Level);
    void log(std::string msg);
private:
    int logLevel_;
    Logger(){}
};
