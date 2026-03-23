/**
 * @brief 使用Epoll构建的Poller
 * @date 2026.03.20
 */

#pragma once

#include "Poller.h"
#include <sys/epoll.h>

class Channel;

/**
 * @brief epoll多路复用Poller
 */
class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
private:
    static const int KInitEventListSize = 16;

    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};
