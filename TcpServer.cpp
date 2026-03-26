/**
 * @brief TcpServer
 * @date 2026.03.23
 */

#include "TcpServer.h"
#include "Logger.h"
#include <strings.h>

static EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

/**
 * @brief 构造函数，，构造后设置新连接产生回调
 * @param loop 传入的EventLoop指针
 * @param listenAddr 传入的监听socket地址
 * @param nameArg 传入的Server名称
 */
TcpServer::TcpServer(EventLoop* loop, 
            const InetAddress& listenAddr, 
            const std::string& nameArg,
            Option option) 
        : loop_(CheckLoopNotNull(loop))
        , ipPort_(listenAddr.toIpPort())
        , name_(nameArg)
        , acceptor_(new Acceptor(loop, listenAddr, option == KReusePort))
        , threadPool_(new EventLoopThreadPool(loop, name_))
        , connectionCallback_()
        , messageCallback_()
        , nextConnId_(1)
        , started_(0) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, 
                                        std::placeholders::_1, std::placeholders::_2));
}

/**
 * @brief 析构函数，遍历所有连接并销毁
 */
TcpServer::~TcpServer() {
    for(auto& item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
    }
}

/**
 * @brief 设置线程数量
 * @param nunThreads 线程数量
 */
void TcpServer::setThreadNum(int numThreads) {
    threadPool_->setThreadNum(numThreads);
}

/**
 * @brief 开启事件循环，创建线程，主loop开启监听
 * @param nunThreads 线程数量
 */
void TcpServer::start() {
    if(started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

/**
 * @brief 新连接处理函数，为其创建TcpConnection，将连接分配给子loop，并设置其回调
 * @param sockfd 新连接fd
 * @param peerAddr 新连接对端socket地址
 */
void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    EventLoop* ioLoop = threadPool_->getNextLoop();

    char buf[64] = { 0 };
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connname = name_ + buf;
    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n", 
                name_.c_str(), connname.c_str(), peerAddr.toIpPort().c_str());

    sockaddr_in local;
    socklen_t addrlen = sizeof local;
    ::bzero(&local, addrlen);
    if(::getsockname(sockfd, (sockaddr*)&local, &addrlen) < 0) LOG_ERROR("sockets::getLocalAddr");
    InetAddress localAddr(local);

    TcpConnectionPtr conn(new TcpConnection(ioLoop, connname, sockfd, localAddr, peerAddr));
    connections_[connname] = conn;

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

/**
 * @brief 删除连接处理函数，将销毁连接函数注册到其queueinloop
 * @param conn 要删除的连接
 */
void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    loop_->runInLoop(std::bind(std::bind(&TcpServer::removeConnectionInLoop, this, conn)));
}

/**
 * @brief 删除连接处理函数，首先从连接列表中删除，随后将销毁链接函数添加到其回调队列
 * @param conn 要删除的连接
 */
void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn) {
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n",
                 name_.c_str(), conn->name().c_str());
    connections_.erase(conn->name());
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestoryed, conn));
}
