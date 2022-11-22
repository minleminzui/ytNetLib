#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include "util.h"


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

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    bzero(&client_addr, sizeof(client_addr));

    int retfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);
    errif(retfd == -1, "socket accept error");

    printf("new client fd %d, its address is %s, its port is %d\n", retfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while (true) {
        char buf[1024];
        bzero(buf, sizeof(buf));
        // bzero(&buf, sizeof(buf));
        ssize_t ret = read(retfd, buf, sizeof(buf));
        if (ret > 0) {
            printf("read from retfd %d: %s\n", retfd, buf);
            write(retfd, buf, sizeof(buf));
        } else if (ret == 0) {
            printf("client fd %d disconnected\n", retfd);
            close(retfd);
            break;
        } else if (ret == -1) {
            close(retfd);
            errif(true, "socket read error\n");
        }
    }
    close(listenfd);
    return 0;
}
