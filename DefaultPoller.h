/**
 * @brief Poller实例的选择性生成
 * @date 2026.03.20
 */

#include "Poller.h"
#include "EPollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop) {
    if(::getenv("MUDUO_USE_POLL")) {
        return nullptr;
    }
    else {
        return nullptr;
    }
}
