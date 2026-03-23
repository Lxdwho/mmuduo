/**
 * @brief 获取线程id
 * @date 2026.03.21
 */

#include "CurrentThread.h"

namespace CurrentThread {
    __thread int t_cachedTid = 0;
    /* 获取调用线程id */
    void cacheTid() {
        if(t_cachedTid == 0) {
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
} // CurrentThread
