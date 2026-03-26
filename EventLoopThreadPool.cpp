/**
 * @brief EventLoop线程池模式
 * @date 2026.03.22
 */

#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg)
        : baseloop_(baseLoop)
        , name_(nameArg)
        , started_(false)
        , numThreads_(0)
        , next_(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

/**
 * @brief 启动线程池，根据numThreads_创建线程，numThreads_==0则只创建baseloop线程
 * @param cb 线程初始化回调，每个线程都需要传入
 */
void EventLoopThreadPool::start(const ThreadLoopInitCallback& cb) {
    started_ = true;
    for(int i=0; i<numThreads_; i++) {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof buf, "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }
    if(numThreads_ == 0 && cb) cb(baseloop_);
}

/**
 * @brief 获取线程对象，numThreads_==0则直接返回baseloop
 * @return 返回线程的eventloop
 */
EventLoop* EventLoopThreadPool::getNextLoop() {
    EventLoop* loop = baseloop_;
    if(!loops_.empty()) {
        loop = loops_[next_];
        ++next_;
        if(next_ >= loops_.size()) next_ = 0;
    }
    return loop;
}

/**
 * @brief 返回所有线程的eventloop
 * @return 返回所有线程的eventloop
 */
std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    if(loops_.empty()) return std::vector<EventLoop*>(1, baseloop_);
    else return loops_;
}
