/**
 * @brief TcpServer
 * @date 2026.03.23
 */

#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"
#include <functional>
#include <unordered_map>

class TcpServer : muduo::noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
    enum Option { KNoReusePort, KReusePort };

    TcpServer(EventLoop* loop, 
              const InetAddress& listenAddr, 
              const std::string& nameArg,
              Option option = KNoReusePort);
    ~TcpServer();
    
    /* 设置线程初始化回调 */
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    /* 设置连接回调 */
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    /* 设置消息回调 */
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    /* 设置写完成回调 */
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

    void setThreadNum(int numThreads);
    void start();
private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;

    const std::string ipPort_;
    const std::string name_;

    std::unique_ptr<Acceptor> acceptor_;

    std::shared_ptr<EventLoopThreadPool> threadPool_;

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;

    ThreadInitCallback threadInitCallback_;
    std::atomic_int started_;
    int nextConnId_;
    ConnectionMap connections_;
};
