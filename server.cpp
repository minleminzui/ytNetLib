#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024


void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
} 

int main() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(listenfd == -1, "socket create error");

    sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8888);

    errif(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    errif(listen(listenfd, SOMAXCONN) == -1, "socket listen error");

    int epfd = epoll_create1(0); // epoll_create1 的参数是一个flag
    errif(epfd == -1, "epoll_create failed");

    struct epoll_event events[MAX_EVENTS], ev;
    bzero(events, sizeof(events));
    bzero(&ev, sizeof(ev));

    ev.events = EPOLLIN;
    ev.data.fd = listenfd;
    setnonblocking(listenfd);
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    while (true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        errif(nfds == -1, "epoll_wait failed");

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listenfd) {
                sockaddr_in client_adr;
                bzero(&client_adr, sizeof(client_adr));
                socklen_t client_adr_len = sizeof(client_adr);
                int client_sockfd = accept(listenfd, (sockaddr*)&client_adr, &client_adr_len);
                errif(client_sockfd == -1, "accept failed");

                printf("New client fd is: %d! and IP is %s, port is %d!\n", client_sockfd, inet_ntoa(client_adr.sin_addr), ntohs(client_adr.sin_port));
                bzero(&ev, sizeof(ev));

                ev.data.fd = client_sockfd;
                ev.events = EPOLLIN | EPOLLET;
                setnonblocking(client_sockfd);
                epoll_ctl(epfd, EPOLL_CTL_ADD, client_sockfd, &ev);
            } else if (events[i].data.fd & EPOLLIN) {
                char buf[READ_BUFFER];
                while (true) { // 非阻塞IO读取数据
                    bzero(buf, sizeof(buf));
                    ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                    if (bytes_read > 0) {
                        printf("message received from fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, bytes_read);
                    } else if (bytes_read == -1 && errno == EINTR) { // 慢系统调用，被信号中断？
                        printf("continue reading");
                        continue;
                    } else if (bytes_read == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) { 
                        printf("this read finished, errno: %d\n", errno);
                        break;
                    } else if (bytes_read == 0) {
                        printf("client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd); // 关闭socket会自动将文件描述符从epoll树上移除
                        break;
                    }
                }
            } else {
                printf("something else happened\n");
            }
        }
    }
    close(listenfd);
    return 0;
}
