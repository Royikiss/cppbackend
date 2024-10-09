#include <iostream>
#include <sys/select.h>
#include <unistd.h>

int main() {
    fd_set readfds;
    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);  // STDIN_FILENO是标准输入的文件描述符

    // 调用select，监视标准输入是否可读，超时时间为5秒
	// 注意我们第一个参数一般是监视的最大文件描述符 + 1
	// 如果要监视多个文件描述符，在得到返回值之后可以使用for循环遍历FD_ISSET判断哪些文件描述符可读
    int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

    if (result == -1) {
        perror("select error");
        return 1;
    } else if (result == 0) {
        std::cout << "No input within 5 seconds.\n";
    } else {
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char buffer[256];
            read(STDIN_FILENO, buffer, sizeof(buffer));
            std::cout << "Input: " << buffer;
        }
    }

    return 0;
}