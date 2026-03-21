/**
 * @brief 事件循环？
 * @date 2026.03.20
 */

#include "EventLoop.h"
#include "Poller.h"
#include "Logger.h"
#include <unistd.h>
#include <sys/eventfd.h>
#include <memory>

__thread EventLoop* t_loopInThisThread = nullptr;

const int KPollTimeMs = 10000;

int createEventfd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtfd < 0) {
        LOG_FATAL("eventfd create error:%d\n", errno);
    }
    return evtfd;
}

EventLoop::EventLoop() 
        : threadId_(CurrentThread::tid())
        , looping_(false)
        , quit_(false)
        , callingPendingFunctors_(false)
        , poller_(Poller::newDefaultPoller(this))
        , wakeupFd_(createEventfd())
        , wakeupChannel_(new Channel(this, wakeupFd_)) {
    LOG_DEBUG("EventLoop create %p in thread %d \n", this, threadId_);
    if(t_loopInThisThread) {
        LOG_FATAL("Anorther EventLoop %p exits in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else t_loopInThisThread = this;
    // wakeupfd对应channel的事件设置
    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handlerRead, this));
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping\n", this);

    while(!quit_) {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(KPollTimeMs, &activeChannels_);
        for(Channel* channel : activeChannels_) {
            channel->handlerEvent(pollReturnTime_);
        }
        doPendingFunctors();
    }

    LOG_INFO("EventLoop %p stop looping.\n", this);
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    // 其他线程中调用quit，需要做唤醒操作？？？？？？？？？？？
    if(!isInLoopThread()) wakeup();
}

void EventLoop::runInLoop(Functor cb) {
    if(isInLoopThread()) cb();  // 在当前loop线程中，直接执行
    else queueInLoop(cb);       // 不在当前loop线程，先存，唤醒后再执行
}

void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);
    }
    if(!isInLoopThread() || callingPendingFunctors_) wakeup();
}

void EventLoop::updateChannel(Channel* channel) { poller_->updateChannel(channel); }
void EventLoop::removeChannel(Channel* channel) { poller_->removeChannel(channel); }
bool EventLoop::hasChannel(Channel* channel) { return poller_->hasChannel(channel); }

void EventLoop::handlerRead() {
    uint64_t one = 1;
    ssize_t n = read(wakeupFd_, &one, sizeof one);
    if(n != sizeof one) {
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8 \n", n);
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof one);
    if(n != sizeof one) {
        LOG_ERROR("EventLoop::wakeup() writes %lu bytes instead of 8 \n", n);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}
