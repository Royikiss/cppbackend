#  IO多路复用

[TOC]

I/O多路复用（I/O multiplexing）是一种能够同时监听多个I/O事件的技术，通常用于处理大量的网络连接或文件描述符。它允许一个单独的线程或进程同时等待多个输入源的数据到达或准备好进行输出，而无需为每个I/O操作创建一个线程或进程。这在高并发网络编程中非常重要，能显著减少资源消耗。

## C++中的相关技术

### select

`select` 是最早的 I/O 多路复用机制，常用于监视多个文件描述符（如套接字、管道、文件等），并等待其中的一个或多个文件描述符变为可读、可写或有异常发生。它的基本思想是通过阻塞等待，直到某个文件描述符变为就绪状态（如可读或可写）。

#### `select` 函数原型

```c
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

- `nfds`：文件描述符的数量，通常设置为最大文件描述符加1。
- `readfds`：用于监视可读事件的文件描述符集合。
- `writefds`：用于监视可写事件的文件描述符集合。
- `exceptfds`：用于监视异常事件的文件描述符集合。
- `timeout`：超时设置。可以设置为 `NULL` 表示无限等待；可以指定秒和微秒，或 `0` 表示立即返回。

#### 返回值
- 返回值是就绪文件描述符的数量（正数）。
- 如果超时且没有任何文件描述符就绪，返回0。
- 如果出错，返回-1，并设置 `errno`。

#### `fd_set` 相关操作

- **初始化集合**：`FD_ZERO(&set);`
- **将文件描述符加入集合**：`FD_SET(fd, &set);`
- **将文件描述符从集合中移除**：`FD_CLR(fd, &set);`
- **检查文件描述符是否在集合中**：`FD_ISSET(fd, &set);`

#### 超时设置
- `struct timeval` 定义了超时时间：
  ```c
  struct timeval {
      long tv_sec;  // 秒
      long tv_usec; // 微秒
  };
  ```
  - 如果传入 `NULL`，表示 `select` 将一直阻塞，直到某个文件描述符准备就绪。
  - 如果 `tv_sec` 和 `tv_usec` 都为 0，`select` 将立即返回，起到非阻塞查询的效果。

#### `select` 的使用步骤
1. 初始化 `fd_set` 集合，用于监视可读、可写、异常的文件描述符。
2. 调用 `select`，并传入对应的 `fd_set` 集合和超时。
3. 检查返回值，使用 `FD_ISSET` 来确定哪个文件描述符已准备好进行 I/O 操作。
4. 根据就绪的文件描述符进行读写操作。

#### 示例代码

以下是一个简单的 `select` 示例，监视标准输入（键盘）是否有输入数据：

```cpp
#include <iostream>
#include <sys/select.h>
#include <unistd.h>

int main() {
    fd_set readfds;
    struct timeval timeout;

    // 初始化timeout为5秒
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    // 清空集合并将标准输入加入可读集合
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);  // STDIN_FILENO是标准输入的文件描述符

    // 调用select，监视标准输入是否可读，超时时间为5秒
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
```

#### 示例解释：
- 该程序使用 `select` 监视标准输入是否有数据输入。
- 如果标准输入可读，程序会读取并打印输入内容；否则，如果超过 5 秒没有输入，程序会超时并输出提示。

#### `select` 的局限性
1. **文件描述符限制**：`select` 能监视的文件描述符数量有限，通常最大为 1024，超出限制可能导致程序崩溃。
2. **效率低**：每次调用 `select` 都需要将整个文件描述符集合传递给内核，并在内核中遍历所有文件描述符，随着文件描述符数量增加，性能下降明显。
3. **不支持边沿触发**：`select` 使用的是电平触发（level-triggered），对于事件频繁的描述符来说，可能会导致大量的系统调用。

在现代高并发场景中，通常使用性能更高的 `epoll` 来替代 `select`。

### poll

`poll` 是一种用于 I/O 多路复用的系统调用，它与 `select` 类似，但克服了一些 `select` 的局限性，比如 `poll` 没有文件描述符数量的限制，并且使用结构体数组来管理多个文件描述符。

#### `poll` 函数原型

```c
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

- `fds`：指向 `pollfd` 结构体数组的指针，用于存储需要监控的文件描述符及其事件类型。
- `nfds`：`fds` 数组的大小，即需要监控的文件描述符数量。
- `timeout`：超时时间，单位是毫秒。
  - `timeout = 0` 表示立即返回（非阻塞）。
  - `timeout = -1` 表示无限期阻塞，直到有文件描述符准备好。

#### `pollfd` 结构体

`pollfd` 是 `poll` 函数使用的一个数据结构，用于描述一个文件描述符及其感兴趣的事件和实际发生的事件。

```c
struct pollfd {
    int fd;        // 文件描述符
    short events;  // 监控的事件类型
    short revents; // 实际发生的事件类型
};
```

- `fd`：要监控的文件描述符。
- `events`：指定感兴趣的事件，可以是以下事件的组合：
  - `POLLIN`：数据可读。
  - `POLLOUT`：数据可写。
  - `POLLERR`：发生错误。
  - `POLLHUP`：挂起事件（通常表示对端关闭连接）。
  - `POLLPRI`：高优先级数据可读（带外数据）。
- `revents`：`poll` 返回时设置的实际事件，可以是上述事件的组合，用于表明文件描述符上发生的事件。

#### `poll` 的返回值
- 返回值为正数：表示已准备好的文件描述符数量。
- 返回 0：超时。
- 返回 -1：出错，并设置 `errno`。

#### `poll` 的使用步骤
1. 初始化 `pollfd` 结构体数组，设置需要监控的文件描述符及感兴趣的事件。
2. 调用 `poll` 函数，等待文件描述符上的事件。
3. 返回后检查每个 `pollfd` 结构体的 `revents` 字段，判断哪些文件描述符已准备好。
4. 根据事件类型执行相应的 I/O 操作。

#### 示例代码

以下是一个简单的 `poll` 示例，监控标准输入（键盘）和一个文件描述符的可读性：

```cpp
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
```

#### 示例解释：
- 该程序创建了一个 `pollfd` 数组来监控两个文件描述符：标准输入和假设的文件描述符 `4`。
- 调用 `poll` 后，如果在 5 秒内有数据可读，程序将读取并输出标准输入的内容，否则会显示超时提示。

#### `poll` 与 `select` 的区别
1. **没有文件描述符数量限制**：`poll` 不像 `select` 那样有 1024 个文件描述符的限制，因此可以处理更多的连接。
2. **性能改进**：`poll` 通过 `pollfd` 结构体数组来管理文件描述符，避免了每次调用都需要重新设置文件描述符集合的开销。
3. **事件检测**：`poll` 使用 `revents` 字段来返回发生的事件类型，相较于 `select` 直接修改传入的集合，代码更加简洁清晰。

#### `poll` 的局限性
尽管 `poll` 比 `select` 提供了改进，但它仍然有一些局限：
1. **线性扫描问题**：`poll` 需要线性遍历整个 `pollfd` 数组来检测事件是否发生，这在文件描述符数量较多时会带来性能瓶颈。
2. **不能保留事件状态**：与 `epoll` 不同，`poll` 每次调用仍需要传入整个文件描述符列表，无法记住上次的状态。

### epoll

`epoll` 是 Linux 内核提供的一种高效的 I/O 多路复用机制，专为大规模并发连接场景设计。相比 `select` 和 `poll`，`epoll` 在性能和可扩展性上都有显著优势，尤其是在处理大量文件描述符时。

#### `epoll` 的基本概念

`epoll` 通过事件通知机制，避免了 `select` 和 `poll` 的线性扫描瓶颈。它通过两个步骤来管理文件描述符：
1. 创建一个 `epoll` 实例（事件表）。
2. 将要监控的文件描述符注册到这个事件表中。

当某个文件描述符的状态发生变化时，`epoll` 会将其标记为就绪状态，程序可以通过查询事件表来获取所有就绪的文件描述符。

#### `epoll` 的三种主要操作函数

1. **`epoll_create1`**
   创建一个 `epoll` 实例（事件表）。

   ```c
   int epoll_create1(int flags);
   ```

   - `flags`：常用的标志是 `EPOLL_CLOEXEC`，在 `fork` 子进程时关闭文件描述符。
   - 返回值是 `epoll` 实例的文件描述符。

2. **`epoll_ctl`**
   向 `epoll` 实例中添加、修改或删除要监控的文件描述符及其事件。

   ```c
   int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
   ```

   - `epfd`：`epoll` 实例的文件描述符。
   - `op`：指定操作类型，可以是以下三种：
     - `EPOLL_CTL_ADD`：将文件描述符添加到 `epoll` 实例中。
     - `EPOLL_CTL_MOD`：修改已存在的文件描述符的监控事件。
     - `EPOLL_CTL_DEL`：从 `epoll` 实例中移除文件描述符。
   - `fd`：要监控的文件描述符。
   - `event`：指向一个 `epoll_event` 结构体，用于指定监控的事件类型。

3. **`epoll_wait`**
   等待事件发生，并获取所有就绪的文件描述符。

   ```c
   int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
   ```

   - `epfd`：`epoll` 实例的文件描述符。
   - `events`：指向一个 `epoll_event` 数组，用于存储就绪的文件描述符。
   - `maxevents`：能够返回的最多就绪事件数量。
   - `timeout`：超时设置，单位为毫秒，`-1` 表示无限等待，`0` 表示立即返回。

#### `epoll_event` 结构体

```c
struct epoll_event {
    uint32_t events;  // 监控的事件类型（如 EPOLLIN、EPOLLOUT 等）
    epoll_data_t data;  // 用户数据，可以是文件描述符或其他类型
};
```

- `events`：可以指定多个事件类型的组合，包括：
  - `EPOLLIN`：可读事件。
  - `EPOLLOUT`：可写事件。
  - `EPOLLERR`：错误事件。
  - `EPOLLRDHUP`：连接关闭事件。
  - `EPOLLET`：启用边沿触发（edge-triggered）模式。
- `data`：用于存储用户自定义数据，通常会存储文件描述符。

#### `epoll` 的使用步骤

1. **创建 `epoll` 实例**：使用 `epoll_create1` 创建一个 `epoll` 事件表。
2. **注册事件**：使用 `epoll_ctl` 向 `epoll` 实例中添加要监控的文件描述符，并指定感兴趣的事件。
3. **等待事件**：使用 `epoll_wait` 阻塞等待文件描述符上发生的事件，返回所有就绪的文件描述符。
4. **处理事件**：遍历返回的事件数组，执行相应的 I/O 操作。
5. **关闭 `epoll` 实例**：使用 `close` 关闭 `epoll` 实例的文件描述符。

#### `epoll` 示例代码

以下是一个简单的 `epoll` 示例，监控标准输入的可读性：

```cpp
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
```

#### 示例解释：
1. 创建一个 `epoll` 实例，获得 `epfd`。
2. 注册标准输入（`STDIN_FILENO`）的可读事件到 `epoll` 实例。
3. 调用 `epoll_wait`，阻塞等待事件发生，当标准输入可读时返回。
4. 当有事件发生时，读取并输出标准输入的内容。
5. 最后关闭 `epoll` 实例。

#### `epoll` 的两种触发模式

1. **水平触发（Level-Triggered, LT）**：
   - 默认模式，类似于 `select` 和 `poll`。
   - 当文件描述符上有事件发生时，`epoll_wait` 会反复返回该事件，直到该事件被处理。

2. **边沿触发（Edge-Triggered, ET）**：
   - 更加高效的模式，但使用起来也更加复杂。
   - 当文件描述符从未就绪到就绪状态时，`epoll_wait` 只会返回一次事件，必须立即处理所有数据，否则可能错过后续事件。

   在边沿触发模式下，用户需要确保在每次事件发生时读尽或写尽文件描述符中的数据，否则会丢失后续的事件通知。

#### `epoll` 的优点

1. **高效性**：`epoll` 的事件驱动模型使其在处理大量文件描述符时性能远超 `select` 和 `poll`，尤其在大规模并发的网络服务器中表现优异。
2. **无需重复注册**：文件描述符只需注册一次，内核会自动记录状态变化，避免了每次调用都重新设置监控对象的开销。
3. **支持边沿触发模式**：在大多数情况下，边沿触发模式下的 `epoll` 比水平触发更高效，因为它避免了重复通知。

#### `epoll` 的局限性
- 仅在 Linux 系统上可用，不能移植到其他操作系统（如 Windows）。
- 复杂度较高，特别是使用边沿触发模式时，需要确保正确处理所有事件数据，否则可能导致事件丢失。

总结来说，`epoll` 是 Linux 系统中处理高并发场景的首选 I/O 多路复用技术，广泛应用于高性能服务器、网络应用等领域。