#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"
#include "Socket.h"
#include "Epoll.h"
#include "InetAddress.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024
void handleReadEvent(int sockfd);

void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
} 

int main() {
    Socket* serv_sock = new Socket();
    // 注意 InetAddress类，由于它的成员变量 struct sockaddr_in addr
    // 传入的ip地址与端口号初始化，那么ip地址必须得有普适性，所以不能够使用htonl()函数
    // 来addr.sin_addr.s_addr = htonl(INADDR_ANY);
    InetAddress* address = new InetAddress("127.0.0.1", 8888);
    serv_sock->bind(address);
    serv_sock->listen();

    Epoll* ep = new Epoll();
    serv_sock -> setnonblocking();
    ep->addFd(serv_sock -> getFd(), EPOLLIN | EPOLLET);

    while (true) {
        std::vector<epoll_event> events = ep -> poll();
        int nfds = events.size();
        for (int i = 0; i < nfds; i++) { 
            if (events[i].data.fd == serv_sock -> getFd()) { // 新客户端连接
                InetAddress* clnt_addr = new InetAddress(); // 这里内存泄漏了，没有delete
                Socket *clnt_sock = new Socket(serv_sock -> accept(clnt_addr)); // 没有delete，内存泄漏
                printf("new client fd %d! IP: %s Port: %d\n", clnt_sock -> getFd(), inet_ntoa(clnt_addr -> addr.sin_addr), ntohs(clnt_addr -> addr.sin_port));
                clnt_sock -> setnonblocking();
                ep -> addFd(clnt_sock -> getFd(), EPOLLIN | EPOLLET);
            } else if (events[i].events & EPOLLIN) {
                handleReadEvent(events[i].data.fd);
            } else {
                printf("something else happened\n");
            }
        }
    }

    delete serv_sock;
    delete address;
    return 0;
}

void handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];

    while (true) { // 由于使用非阻塞IO，读取客户端buffer，一次读取buf大小数据，直到全部读取完毕
        bzero(&buf, sizeof(buf));
        ssize_t  bytes_read = read(sockfd, buf, sizeof(buf));
        if (bytes_read > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (bytes_read == -1 && errno == EINTR) { // 客户端正常中断，继续读取
            printf("continue reading");
            continue;
        } else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) { // 非阻塞IO，这个条件表示数据读取完毕
            printf("finish reading once, errno: %d\n", errno);
            break;
        } else if (bytes_read == 0) { // EOF 客户端断开连接
            printf("EOF, client fd %d disconnected\n", sockfd);
            close(sockfd); // 关闭socket会自动将文件描述符从epoll树上移除
            break;
        }
    } 
}