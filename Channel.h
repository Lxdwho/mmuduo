/**
 * @brief Channel类封装了socket及其感兴趣的event
 * @date 2026.03.20
 */

#pragma once

#include "noncopyable.h"
#include "Timestamp.h"
#include <functional>
#include <memory>

class EventLoop;

class Channel : muduo::noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handlerEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    void tie(const std::shared_ptr<void>&);

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    void enableReading()  { events_ |=  KReadEvent ; update(); }
    void enablewriting()  { events_ |=  KWriteEvent; update(); }
    void disableReading() { events_ &= ~KReadEvent ; update(); }
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    void disableAll()     { events_  =  KNoneEvent ; update(); }

    bool isNoneEvent() const { return events_ == KNoneEvent; }
    bool isWriting() const { return events_ & KWriteEvent; }
    bool isReading() const { return events_ & KReadEvent; }

    int index() { return index_; }
    void set_index (int index) { index_ = index; }

    EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    static const int KNoneEvent;
    static const int KReadEvent;
    static const int KWriteEvent;

    EventLoop* loop_;   // 事件循环
    const int fd_;      // Poller监听对象
    int events_;        // 注册fd感兴趣的事情
    int revents_;       // Poller返回的具体发生的事件
    int index_;

    std::weak_ptr<void> tie_;
    bool tied_;

    // 具体事件回调
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};


