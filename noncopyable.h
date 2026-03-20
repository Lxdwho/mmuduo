/**
 * @brief 禁止拷贝、赋值操作的基类
 * @date 2026.03.19
 */

#pragma once

namespace muduo {

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

} // muduo
