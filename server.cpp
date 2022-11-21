#include <cstdio>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8888);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(listenfd, 5);

    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    bzero(&client_addr, sizeof(client_addr));
    int retfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_len);

    printf("new client fd %d, its address is %s, its port is %d\n", retfd, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    return 0;
}
