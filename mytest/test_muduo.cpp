/**
 * @brief 测试一下muduo库
 * @date 2026.03.19
 */

#include <functional>
#include <iostream>
#include <string>
#include "muduo-master/muduo/net/TcpServer.h"
#include "muduo-master/muduo/net/EventLoop.h"

using namespace muduo::net;
using namespace muduo;

/**
 * 1.创建TcpServer对象
 * 2.创建EventLoop事件循环对象指针
 * 3.明确TcpServer构造参数需要什么参数，输出ChatServer的构造函数
 * 4.在当前服务器的构造函数中，注册处理连接的回调函数以及处理事件的回调函数
 * 5.设置合适的服务端线程数量，muduo库会自己划分IO线程以及worker线程
 */

class ChatServer {
public:
    ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& nameAgr)
                : server_(loop, listenAddr, nameAgr), loop_(loop) {
		// 设置处理连接的回调函数
        server_.setConnectionCallback(
				std::bind(&ChatServer::onConnection, this, std::placeholders::_1));
		// 设置处理事件的回调函数
		server_.setMessageCallback(std::bind(&ChatServer::onMessage, this, 
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		// 设置服务器端的线程数量
		server_.setThreadNum(4);
    }
	
	// 开启事件循环
	void start() {
		server_.start();
	}

private:
	// 用于处理用户的连接与断开
	void onConnection(const TcpConnectionPtr& conn) {
		if(conn->connected()) {
			std::cout << conn->peerAddress().toIpPort() << "->"
					  << conn->localAddress().toIpPort() << "state:online" << std::endl;
		}
		else {
			std::cout << conn->peerAddress().toIpPort() << "->" 
					  << conn->localAddress().toIpPort() << "state:offline" << std::endl;
            conn->shutdown();
            // loop_->quit();
		}
	}
	
	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
		std::string buffer = buf->retrieveAllAsString();
		std::cout << "recv data: " << buffer << "time: " << time.toString() << std::endl;
		conn->send(buffer);
	}
    TcpServer server_;
    EventLoop* loop_;
};

int main() {
    EventLoop loop;
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChartServer");

    server.start();
    loop.loop();
}
