/**
 * @brief 简单日志类
 * @date 2026.03.19
 */

#include "Logger.h"
#include "Timestamp.h"
#include <iostream>

Logger& Logger::Instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int Level) {
    logLevel_ = Level;
}

/**
 * @brief 日志打印函数
 * @param msg 输出的信息
 */
void Logger::log(std::string msg) {
    switch (logLevel_) {
        case INFO:  std::cout << "[INFO]";break;
        case ERROR: std::cout << "[ERROR]";break;
        case FATAL: std::cout << "[FATAL]";break;
        case DEBUG: std::cout << "[DEBUG]";break;
        default: break;
    }
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;
}
