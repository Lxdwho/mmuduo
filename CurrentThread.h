/**
 * @brief 获取线程id
 * @date 2026.03.21
 */

#pragma once

#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {

    extern __thread int t_cachedTid;

    void cacheTid();
    /* 返回调用线程id */
    inline int tid() {
        if(__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();
        }
        return t_cachedTid;
    }
} // CurrentThread
