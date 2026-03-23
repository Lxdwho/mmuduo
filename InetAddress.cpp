/**
 * @brief Socket地址封装类
 * @date 2026.03.19
 */

#include "InetAddress.h"
#include <strings.h>
#include <cstring>

InetAddress::InetAddress(uint64_t port, std::string ip) {
    bzero(&addr_, sizeof addr_);
    addr_.sin_port = htons(port);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = inet_addr(ip.c_str());
}

/**
 * @brief 输出socket地址的ip
 * @return 点分十进制ip
 */
std::string InetAddress::toIp() const {
    char buf[64] = { 0 };
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    return buf;
}

/**
 * @brief 输出socket地址的ip以及port
 * @return 点分十进制ip:port
 */
std::string InetAddress::toIpPort() const {
    char buf[64] = { 0 };
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof buf);
    size_t end = strlen(buf);
    uint16_t port = ntohs(addr_.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

/**
 * @brief 输出socket地址的port
 * @return port uint16_t
 */
uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);
}
