#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1 failed");
        return 1;
    }

    struct epoll_event ev, events[10];
    ev.events = EPOLLIN;  // 监控可读事件
    ev.data.fd = STDIN_FILENO;  // 标准输入文件描述符

    // 注册标准输入到epoll事件表
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
        perror("epoll_ctl failed");
        return 1;
    }

    // 等待事件发生
    int nfds = epoll_wait(epfd, events, 10, -1);
    if (nfds == -1) {
        perror("epoll_wait failed");
        return 1;
    }

    // 遍历就绪的文件描述符
    for (int i = 0; i < nfds; i++) {
        if (events[i].data.fd == STDIN_FILENO) {
            char buffer[256];
            ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (count == -1) {
                perror("read error");
            } else {
                buffer[count] = '\0';
                std::cout << "Input: " << buffer;
            }
        }
    }

    close(epfd);  // 关闭epoll文件描述符
    return 0;
}