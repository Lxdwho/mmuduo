#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
// #include "../../../../1_tcp_socket/1_fuc_tcp/wrap.h"

#define SERV_PORT 1520
#define MAX_WORD 80
#define MAX_OPEN 1024

int main(int argc, char* argv[])
{
    int listen_fd, efd, ret, nready, connect_fd, sockfd, n;
    struct sockaddr_in serv_addr, client_addr;
    struct epoll_event tep, ep[MAX_OPEN];
    socklen_t client_addr_len;
    char str[INET_ADDRSTRLEN], buf[MAX_WORD];

    listen_fd = Socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器Socket地址
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // 绑定到监听的fd，开始监听
    Bind(listen_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    Listen(listen_fd, 20);

    // 创建epoll，将监听的Socket的fd给到epoll
    efd = epoll_create(MAX_OPEN);
    if(efd < 0) perr_exit("epoll_creat");
    tep.data.fd = listen_fd;
    tep.events = EPOLLIN;
    ret = epoll_ctl(efd, EPOLL_CTL_ADD, listen_fd, &tep);
    if(ret < 0) perr_exit("epoll_ctr");

    while(1){
        nready = epoll_wait(efd, ep, MAX_OPEN, -1);
        if(nready < 0) perr_exit("epoll_wait");
        for(int i=0;i<nready;i++){
            if(!(ep[i].events & EPOLLIN)) continue;
            if(ep[i].data.fd == listen_fd){ // 如果是监听套接字有消息，执行连接逻辑
                client_addr_len = sizeof(client_addr);
                connect_fd = Accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
                printf("Client connected! IP is %s, Port is %d\n", 
                        inet_ntop(AF_INET, &client_addr.sin_addr, str, INET_ADDRSTRLEN), 
                        ntohs(client_addr.sin_port));
                tep.events = EPOLLIN;
                tep.data.fd = connect_fd;
                ret = epoll_ctl(efd, EPOLL_CTL_ADD, connect_fd, &tep);
                if(ret < 0) perr_exit("epoll_ctl");
            }
            else{ // 如果是连接套接字有消息，执行回显逻辑
                sockfd = ep[i].data.fd;
                n = Read(sockfd, buf, MAX_WORD);
                if(n == 0){
                     ret = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                     if(ret == -1) perr_exit("epoll_ctr");
                     Close(sockfd);
                     printf("Client Closed: fd is %d\n", ep[i].data.fd);
                }
                else{
                    for(int j=0;j<n;j++){
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf, n);
                }
            }
        }
    }
    Close(listen_fd);
    Close(efd);
    return 0;
}
