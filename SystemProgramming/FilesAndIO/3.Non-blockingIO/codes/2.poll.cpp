#include <iostream>
#include <poll.h>
#include <unistd.h>

int main() {
    struct pollfd fds[2];  // 创建两个pollfd，分别监控标准输入和文件描述符
    int timeout_ms = 5000; // 超时时间5秒

    // 监控标准输入的可读性
    fds[0].fd = STDIN_FILENO;  // 标准输入
    fds[0].events = POLLIN;    // 监控可读事件

    // 监控一个假设的文件描述符（这里假设为4）
    fds[1].fd = 4;        // 假设的文件描述符
    fds[1].events = POLLIN;  // 监控可读事件

    int ret = poll(fds, 2, timeout_ms);

    if (ret == -1) {
        perror("poll error");
        return 1;
    } else if (ret == 0) {
        std::cout << "Timeout occurred! No data within 5 seconds.\n";
    } else {
        // 检查标准输入是否可读
        if (fds[0].revents & POLLIN) {
            char buffer[256];
            read(STDIN_FILENO, buffer, sizeof(buffer));
            std::cout << "Input: " << buffer;
        }

        // 检查文件描述符4是否可读
        if (fds[1].revents & POLLIN) {
            std::cout << "File descriptor 4 is ready for reading.\n";
            // 可以执行读取操作
        }
    }

    return 0;
}