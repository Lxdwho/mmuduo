/**
 * @brief Poller实例的选择性生成
 * @date 2026.03.20
 */

#include "Poller.h"
#include "EPollPoller.h"

/**
 * @brief 根据环境变量，选择返回epoll、poll中的一种
 * @param loop poller对应的事件循环
 */
Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if(::getenv("MUDUO_USE_POLL")) {
        return nullptr;
    }
    else {
        return new EpollPoller(loop);
    }
}
