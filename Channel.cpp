/**
 * @brief Channel类封装了socket及其感兴趣的event
 * @date 2026.03.20
 */

#include "Channel.h"
#include "Logger.h"
#include "EventLoop.h"
#include <sys/epoll.h>

const int Channel::KNoneEvent = 0;
const int Channel::KReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::KWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
    : loop_(loop), fd_(fd), events_(0), revents_(0), index_(-1), tied_(false) {}
Channel::~Channel() {}

/**
 * @brief 处理发生的事件
 * @param receiveTime 发生时间
 */
void Channel::handlerEvent(Timestamp receiveTime) {
    if(tied_) {
        std::shared_ptr<void> guard = tie_.lock(); // weak_ptr!!!!
        if(guard) handleEventWithGuard(receiveTime);
    }
    else {
        handleEventWithGuard(receiveTime);
    }
}

/**
 * @brief 设置tie指针
 * @param obj 对应的EventLoop智能指针
 */
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

/**
 * @brief 从channel所属的EventLoop中删除
 */
void Channel::remove() {
    loop_->removeChannel(this);
}

/**
 * @brief 更新channel状态：加入、删除、修改
 */
void Channel::update() {
    loop_->updateChannel(this);
}

/**
 * @brief 根据发生事件类型调用对应回调
 * @param receiveTime 发生时间
 */
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_INFO("channel handlerEvent revents:%d\n", revents_);
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
        if(closeCallback_)  closeCallback_();
    
    if(revents_ & EPOLLERR)
        if(errorCallback_) errorCallback_();

    if(revents_ & (EPOLLIN | EPOLLPRI))
        if(readCallback_) readCallback_(receiveTime);

    if(revents_ & EPOLLOUT)
        if(writeCallback_) writeCallback_();
}
