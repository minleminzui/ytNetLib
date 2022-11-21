#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdio>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8888);

    if (connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        printf("connect failed\n");
        return 0;
    }
}