/**
 * @brief 多路事件分发器中的核心IO复用模块基类
 * @date 2026.03.20
 */

#include "Poller.h"

Poller::Poller(EventLoop* loop) : ownerLoop_(loop) { }

bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}
