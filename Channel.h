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

/**
 * @brief 封装了socket及其感兴趣的event
 */
class Channel : muduo::noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handlerEvent(Timestamp receiveTime);

    /* 设置读回调 */
    void setReadCallback(ReadEventCallback cb)  { readCallback_ = std::move(cb); }
    /* 设置写回调 */
    void setWriteCallback(EventCallback cb)     { writeCallback_ = std::move(cb); }
    /* 设置关闭回调 */
    void setCloseCallback(EventCallback cb)     { closeCallback_ = std::move(cb); }
    /* 设置错误回调 */
    void setErrorCallback(EventCallback cb)     { errorCallback_ = std::move(cb); }

    void tie(const std::shared_ptr<void>&);

    /* 返回channel对应fd */
    int fd() const { return fd_; }
    /* 返回channel感兴趣的事件 */
    int events() const { return events_; }
    /* 设置channel对应fd发生的事件 */
    void set_revents(int revt) { revents_ = revt; }

    /* 让channel对读感兴趣，并在所属loop中使能 */
    void enableReading()  { events_ |=  KReadEvent ; update(); }
    /* 让channel对写感兴趣，并在所属loop中使能 */
    void enablewriting()  { events_ |=  KWriteEvent; update(); }
    /* 让channel对读不感兴趣，并在所属loop中使能 */
    void disableReading() { events_ &= ~KReadEvent ; update(); }
    /* 让channel对写不感兴趣，并在所属loop中使能 */
    void disableWriting() { events_ &= ~KWriteEvent; update(); }
    /* 让channel对所有事件不感兴趣，并在所属loop中使能 */
    void disableAll()     { events_  =  KNoneEvent ; update(); }

    /* 返回channel是否对所有事件不感兴趣 */
    bool isNoneEvent() const { return events_ == KNoneEvent; }
    /* 返回channel是否对写事件感兴趣 */
    bool isWriting() const { return events_ & KWriteEvent; }
    /* 返回channel是否对读事件感兴趣 */
    bool isReading() const { return events_ & KReadEvent; }

    /* 返回channel的index：加入、删除、修改 */
    int index() { return index_; }
    /* 设置channel的index，标记channel状态：加入、删除、修改 */
    void set_index (int index) { index_ = index; }

    /* 返回channel对应的EventLoop */
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
