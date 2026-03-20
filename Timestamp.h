/**
 * @brief 简易时间类
 * @date 2026.03.19
 */

#pragma once

#include <iostream>
#include <string>

class Timestamp {
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
private:
    int64_t microSecondsSinceEpoch_;
};
