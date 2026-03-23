/**
 * @brief 线程封装类
 * @date 2026.03.21
 */

#include "Thread.h"
#include <semaphore.h>
#include "CurrentThread.h"

Thread::Thread(ThreadFunc func, const std::string& name)
        : started_(false) 
        , joined_(false)
        , tid_(0)
        , func_(std::move(func))
        , name_(name) {
    setDefaultName();
}

Thread::~Thread() {
    if(started_ && !joined_) {
        thread_->detach();
    }
}

/**
 * @brief 线程启动，启动完成后退出
 */
void Thread::start() {
    started_ = true;
    sem_t sem;
    sem_init(&sem, false, 0);

    thread_ = std::shared_ptr<std::thread>(new std::thread([&](){
        tid_ = CurrentThread::tid();
        sem_post(&sem);
        func_();
    }));
    sem_wait(&sem);
}

/**
 * @brief 线程join
 */
void Thread::join() {
    joined_ = true;
    thread_->join();
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if(name_.empty()) {
        char buf[32] = { 0 };
        snprintf(buf, sizeof buf, "Thread%d", num);
        name_ = buf;
    }
}
