/**
 * @brief 多路事件分发器中的核心IO复用模块基类
 * @date 2026.03.20
 */

#pragma once

#include "noncopyable.h"
#include <vector>
#include <unordered_map>
#include "Channel.h"

/**
 * @brief 多路IO复用基类
 */
class Poller : muduo::noncopyable {
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;

    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;
    virtual void updateChannel(Channel* channel) = 0;
    virtual void removeChannel(Channel* Channel) = 0;

    bool hasChannel(Channel* Channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);
protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;
private:
    EventLoop* ownerLoop_;
};
