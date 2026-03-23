/**
 * @brief 
 * @date 2026.03.22
 */

#include "TcpConnection.h"
#include "Timestamp.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"
#include "Socket.h"
#include <sys/socket.h>

/**
 * @brief 检查eventloop是否为空
 * @param loop 待检查的eventloop
 */
static EventLoop* CheckLoopNotNull(EventLoop* loop) {
    if(loop == nullptr) {
        LOG_FATAL("%s:%s:%d TcpConnection Loop is Null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop* loop, 
                const std::string& name, 
                int sockfd, 
                const InetAddress& localAddr,
                const InetAddress& peerAddr) 
        : loop_(CheckLoopNotNull(loop))
        , name_(name)
        , state_(KConnecting)
        , reading_(true)
        , socket_(new Socket(sockfd))
        , channel_(new Channel(loop, sockfd))
        , localAddr_(localAddr)
        , peerAddr_(peerAddr_)
        , highWaterMark_(64*1024*1024) {
    /*==========================================*/
    channel_->setReadCallback (std::bind(&TcpConnection::handleRead , this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    LOG_INFO("TcpConnection::ctor[%s] at fd=%d\n", name_.c_str(), sockfd);
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO("TcpConnection::dtor[%s] at fd=%d state=%d \n", name_.c_str(), channel_->fd(), (int)state_);
}

/**
 * @brief 
 * @param loop 待检查的eventloop
 */
void TcpConnection::send(const std::string& buf) {
    if(state_ == KConnected) {
        if(loop_->isInLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        }
        else {
            loop_->runInLoop(std::bind(&TcpConnection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

/**
 * @brief 关闭连接，向loop中添加shutdownInLoop
 */
void TcpConnection::shutdown() {
    if(state_ == KConnected) {
        setState(KDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

/**
 * @brief 建立连接，使能channel读，执行连接回调
 */
void TcpConnection::connectEstablished() {
    setState(KConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();
    connectioncallback_(shared_from_this());
}

/**
 * @brief 销毁连接，使能channel所有事件，执行连接回调，在loop中remove channel
 */
void TcpConnection::connectDestoryed() {
    if(state_ == KConnected) {
        setState(KDisconnected);
        channel_->disableAll();
        ConnectionCallback(shared_from_this());
    }
    channel_->remove();
}

/**
 * @brief 读事件处理，从fd读到inputbuffer，调用消息回调
 */
void TcpConnection::handleRead(Timestamp timestamp) {
    int saveErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &saveErrno);
    if(n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, timestamp);
    }
    else if(n == 0) {
        handleClose();
    }
    else {
        errno = saveErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
}

/**
 * @brief 写事件处理，将output中的数据写到fd，写完毕调用写完毕回调
 */
void TcpConnection::handleWrite() {
    if(channel_->isWriting()) {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if(n > 0) {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if(writeCompleteCallback_) {
                    loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == KDisconnecting) shutdownInLoop();
            }
        }
        else {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else {
        LOG_ERROR("TcpConnection fd=%d is down, no more writing \n", channel_->fd());
    }
}

/**
 * @brief 关闭事件处理，失能channel所有事件，调用连接、关闭回调
 */
void TcpConnection::handleClose() {
    LOG_INFO("TcpConnection::handleClose fd=%d state=%d \n", channel_->fd(), (int)state_);
    setState(KDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr(shared_from_this());
    connectioncallback_(connPtr);
    closeCallback_(connPtr);
}

/**
 * @brief 错误事件处理，获取错误码，打日志
 */
void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof optval;
    int err = 0;
    if(::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        err = errno;
    }
    else {
        err = optval;
    }
    LOG_ERROR("TcpConnection::handleError name%s - SO_ERROR:%d \n", name_.c_str(), err);
}

/**
 * @brief 向connection对应channel的fd发生数据，一次没写完则使能channel写事件
 * @param message 数据内容
 * @param len 数据长度
 */
void TcpConnection::sendInLoop(const void* message, size_t len) {
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if(state_ == KDisconnected) {
        LOG_ERROR("disconnected, give up writing!");
        return;
    }
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), message, len);
        if(nwrote >= 0) {
            remaining = len - nwrote;
            if(remaining == 0 && writeCompleteCallback_) {
                loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if(errno != EWOULDBLOCK) {
                LOG_ERROR("TcpConnection::sendInLoop");
                if(errno == EPIPE || errno == ECONNREFUSED) {
                    faultError = true;
                }
            }
        }
    }

    if(!faultError && remaining > 0) {
        size_t oldlen = outputBuffer_.readableBytes();
        if(oldlen + remaining >= highWaterMark_ && oldlen < highWaterMark_ && highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldlen + remaining));
        }
        outputBuffer_.append((char*)message + nwrote, remaining);
        if(!channel_->isWriting()) {
            channel_->enablewriting();
        }
    }
}

/**
 * @brief 在channel无写事件时，关闭socket写端
 */
void TcpConnection::shutdownInLoop() {
    if(!channel_->isWriting()) socket_->shutdownWrite();
}
