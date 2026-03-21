/**
 * @brief EventLoop运行的Thread封装
 * @date 2026.03.21
 */

#pragma once

#include "noncopyable.h"
#include "Thread.h"
#include "InetAddress.h"
#include <mutex>
#include <condition_variable>
#include <functional>

class EventLoop;

class EventLoopThread : muduo::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(), 
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();
private:
    void threadFunc();

    EventLoop* loop_;
    bool exiting_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    ThreadInitCallback callback_;
};
