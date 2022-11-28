#include "Socket.h"
#include "InetAddress.h"
#include "util.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socker create error");
}

Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socker create error");
}

Socket::~Socket() {
    if (fd!= -1) {
        close(fd);
        fd = -1;
    }
}


void Socket::bind(InetAddress* inetaddr) {
    errif(::bind(fd, (struct sockaddr*)&(inetaddr -> addr), inetaddr -> addr_len) == -1, "socket bind error");
}

void Socket::listen() {
    errif(::listen(fd, SOMAXCONN), "socket listen error");
}
void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int Socket::accept(InetAddress* inetaddr) {
    int client_sockfd = ::accept(fd, (struct sockaddr*)&inetaddr ->addr, &(inetaddr -> addr_len));
    errif(client_sockfd == -1, "socket accept failed");
    return client_sockfd;
}

int Socket::getFd() {
    return fd;
}