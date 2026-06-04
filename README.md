# CReactorServer

[![C++17](https://img.shields.io/badge/C++-17/20-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)](https://linux.org/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> 一个基于 **Reactor 模式** 的轻量级、高性能 C++ TCP 网络服务端框架。

---

## 📖 目录

- [总览](#总览)
  - [项目概述](#项目概述)
  - [应用场景](#应用场景)
  - [架构总览](#架构总览)
- [细节](#细节)
  - [核心模块](#核心模块)
  - [技术创新](#技术创新)
  - [核心设计](#核心设计)
  - [协议设计](#协议设计)
  - [构建与使用](#构建与使用)

---

## 总览

### 项目概述

**CReactorServer** 是一个从零实现、不依赖任何第三方网络库的 C++ 网络服务端框架。它的设计灵感来源于业界知名的 **Muduo** 网络库（陈硕），核心思想是 **Reactor（反应器）事件驱动模型** + **One Loop Per Thread（一线程一循环）**。

项目完全基于 Linux 系统调用（`epoll`/`poll`、`eventfd`、`readv`、`accept4` 等）构建，实现了完整的非阻塞 I/O 事件循环、多线程并发处理、用户态缓冲区管理、定时器调度和自定义二进制应用层协议。同时通过 `HandlerProxyBasic` 接口将网络层与业务逻辑解耦，使用者只需实现该接口即可快速搭建高性能网络应用。

### 应用场景

CReactorServer 适用于各类 **长连接、高并发、低延迟** 的 C/S 网络服务场景，典型应用包括：

| 场景 | 说明 |
|------|------|
| 🎮 **游戏服务器** | MMORPG、实时对战等需要大量并发 TCP 长连接的场景 |
| 📱 **即时通讯（IM）** | 消息推送、聊天服务器，保持海量客户端在线 |
| 📡 **物联网（IoT）网关** | 设备接入层，处理大量终端设备的 TCP 数据上报 |
| 🔄 **RPC 服务框架底层** | 作为微服务间通信的传输层基石 |
| 💹 **金融交易网关** | 对延迟敏感的行情推送、交易委托服务 |
| 🖥️ **代理/网关服务** | 流量代理、协议转换、负载均衡的前置网关 |

### 架构总览

```
┌─────────────────────────────────────────────────────────────────┐
│                         Application Layer                       │
│                    (HandlerProxyBasic 实现业务逻辑)               │
├─────────────────────────────────────────────────────────────────┤
│                     TcpServer (主Reactor)                        │
│           监听 accept / 连接分发 / 定时器清理 / 单例              │
├─────────────────────────────────────────────────────────────────┤
│               EventLoopThreadPool (I/O 线程池)                   │
│         Round-Robin 分发 → 多个 EventLoopThread                  │
├──────────────┬──────────────┬──────────────┬────────────────────┤
│ EventLoop #1 │ EventLoop #2 │ EventLoop #3 │  ...  EventLoop #N │
│  (工作线程)   │  (工作线程)   │  (工作线程)   │     (工作线程)      │
├──────┴───────┴──────┴───────┴──────┴───────┴──────┴────────────┤
│                        Poller 抽象层                             │
│                  ┌── EpollPoller (epoll, 默认)                   │
│                  └── PollPoller  (poll,  可选)                    │
├─────────────────────────────────────────────────────────────────┤
│                  TcpSocketHandler (每条连接)                      │
│   Buffer(读写缓冲区) + PacketStreamParser(协议解析) + HandlerProxy│
├─────────────────────────────────────────────────────────────────┤
│                     TimerList (定时器管理)                        │
│                    最小堆实现 + 单例模式                           │
└─────────────────────────────────────────────────────────────────┘
```

---

## 细节

### 核心模块

#### 1️⃣ EventLoop — 事件循环引擎

`EventLoop` 是整个框架的心脏。每个 `EventLoop` 唯一绑定一个线程，遵循 **"One Loop Per Thread"** 原则。

**核心流程：**

```
loop()
  ├─ poller_->poll(timeout, &activeObjs)  // 等待 I/O 事件
  ├─ 遍历 activeObjs 调用 ProcessPollerEvents() // 处理 I/O 事件
  ├─ timer_list_->CheckTimerExpired()     // 定时器超时检测
  └─ doPendingFunctors()                  // 执行跨线程提交的任务
```

**跨线程任务投递机制：**

```cpp
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        cb();                    // 同一线程，直接执行
    } else {
        queueInLoop(cb);         // 其他线程，入队 + 唤醒
    }
}
```

当其他线程需要向某个 `EventLoop` 投递任务时，先将任务放入 `pendingFunctors_` 队列，然后向 `eventfd` 写入数据以唤醒目标线程。目标线程被唤醒后在每次循环末尾消费整个任务队列（使用 `swap` 技巧避免长时间持有锁）。

#### 2️⃣ Poller — I/O 多路复用抽象

`Poller` 是抽象基类，提供统一的 I/O 事件监听接口，支持两种后端实现：

| 后端 | 系统调用 | 优缺点 | 启用方式 |
|------|---------|--------|---------|
| **EpollPoller** | `epoll_create` / `epoll_ctl` / `epoll_wait` | 高性能，适合大量 fd | 默认 |
| **PollPoller** | `poll()` | 兼容性好，适合少量 fd | 编译时定义 `_POLL` |

通过 `Poller::newDefaultPoller()` 工厂方法创建默认实例：

```cpp
Poller *Poller::newDefaultPoller() {
#ifdef _POLL
    return new PollPoller();
#endif
    return new EpollPoller();
}
```

`EpollPoller` 的 `event_list_` 支持**自动扩容**：当单次 `epoll_wait` 返回的事件数达到当前容量时，自动翻倍扩容。

#### 3️⃣ PollerObject — 事件驱动对象基类

所有需要监听 I/O 事件的对象（如 `TcpServer`、`TcpSocketHandler`、`WakeUpObject`）都继承自 `PollerObject`。

**事件回调接口（纯虚函数）：**

| 回调 | 触发条件 | 典型实现 |
|------|---------|---------|
| `OnInputNotify()` | 可读事件 | 读取数据、协议解析 |
| `OnOutputNotify()` | 可写事件 | 发送缓冲区数据 |
| `OnCloseNotify()` | 连接关闭/错误 | 清理资源、通知业务层 |
| `OnErrorNotify()` | 异常事件 | 关闭连接 |

**事件注册方法：**

```cpp
void EnableReading(bool enable);   // 注册/取消 可读事件
void EnableWriting(bool enable);   // 注册/取消 可写事件
void DisableAll();                 // 取消所有事件
void DisableAndRemove();           // 从 Poller 中移除
```

`WakeUpObject` 是 `PollerObject` 的子类，用于实现 `EventLoop` 的自唤醒——当收到 `eventfd` 的可读事件时，读取数据清除事件标志，不做额外处理。

#### 4️⃣ TcpServer — TCP 服务端

`TcpServer` 作为整个服务的入口，职责包括：

- **监听套接字管理**：创建 `SOCK_NONBLOCK | SOCK_CLOEXEC` 的非阻塞 socket，绑定地址并监听
- **连接接受**：在 `OnInputNotify()` 中调用 `accept4()` 接受新连接
- **连接分发**：通过 `EventLoopThreadPool::getNextLoop()` 以 **Round-Robin** 方式将新连接分发给工作线程
- **定时清理**：每 5 秒触发定时器，清理已断开连接的 `TcpSocketHandler`
- **单例模式**：继承 `CSingleton<TcpServer>`，全局唯一实例

#### 5️⃣ EventLoopThreadPool — 线程池

`EventLoopThreadPool` 负责创建和管理多个 I/O 工作线程：

```
TcpServer (主线程, 监听)
    │
    │ OnInputNotify → accept → getNextLoop()
    │
    ├── Round-Robin ──────────────────────────┐
    │                                         │
    ▼  (连接1)          ▼  (连接2)          ▼  (连接3)
EventLoopThread #1  EventLoopThread #2  EventLoopThread #3
    │                    │                    │
    ▼                    ▼                    ▼
TcpSocketHandler    TcpSocketHandler     TcpSocketHandler
```

每个 `EventLoopThread` 内部启动一个独立线程，该线程运行一个 `EventLoop::loop()`。主线程通过 `condition_variable` 等待子线程初始化完成并返回 `EventLoop*`。

#### 6️⃣ TcpSocketHandler — 连接处理器

每个 TCP 连接对应一个 `TcpSocketHandler`，负责：

- **数据读取**：`OnInputNotify()` 中从内核缓冲区读取数据到 `inputBuffer_`
- **协议解析**：调用 `PacketStreamParser` 从 `inputBuffer_` 中提取完整数据包
- **数据发送**：`send()` 先尝试直接 `write()`，写不完则启用可写事件等待回调
- **业务回调**：通过 `HandlerProxyBasic` 接口将业务逻辑完全解耦

**关键设计 — 数据读取的策略抉择：**

有两种策略可选（代码中采用了策略二）：

> **策略一**：每次事件响应只处理一个完整包，避免长时间占用 CPU
> → 问题：后续没有事件触发时，剩余数据包会被搁置
>
> **策略二（✓）**：用 `while(true)` 在当前事件中尽力处理所有完整包
> → 优势：一次事件尽量榨干所有完整包，避免搁置问题

#### 7️⃣ Buffer — 高效用户态缓冲区

`Buffer` 的设计借鉴了 Muduo，采用三段式布局：

```
+-------------------+------------------+------------------+
| prependable bytes |  readable bytes  |  writable bytes  |
|                   |     (CONTENT)    |                  |
+-------------------+------------------+------------------+
|                   |                  |                  |
0      <=      readerIndex   <=   writerIndex     <=    size
```

**空间管理策略：**

```cpp
void makeSpace(size_t len) {
    if (可写空间 + 已读前置空间 < len + kCheapPrepend) {
        // ① 扩容：直接 resize
        buffer_.resize(writerIndex_ + len);
    } else {
        // ② 内部挪移：将未读数据前移，腾出尾部空间
        std::copy(begin() + readerIndex_, begin() + writerIndex_,
                  begin() + kCheapPrepend);
    }
}
```

**零拷贝读取 — scatter/gather I/O：**

`readFd()` 使用 `readv()` 系统调用，通过两个 `iovec` 实现高效读取——先填满 `buffer_` 中的可写空间，若还不够则使用栈上临时缓冲区 `extrabuf`，避免每次读取都需预判大小。

#### 8️⃣ PacketStreamParser — 应用层协议解析器

定义了自定义的二进制应用层协议：

```
 0                   4           6
+-------------------+-----------+
|   packet len      |magic code |
+-----------+-------+-----------+
|                               |
+          body bytes           +
|            ... ...            |
+-------------------------------+

packet len  = magic code + body bytes  (网络字节序)
magic code = "XX" (0x58, 0x58)
```

**解析流程：**

```cpp
parse_packet_length(buffer):
  1. less than sizeof(PacketHeader)? → return 0 (继续接收)
  2. magic_code ≠ "XX"?              → return -1 (数据错误)
  3. packet too big?                  → return -1 (数据错误)
  4. incomplete packet?               → return 0 (继续接收)
  5. complete packet received         → return pkg_len
```

#### 9️⃣ Timer / TimerList — 定时器系统

- **Timer**：单个定时器，支持一次性 / 循环定时
- **TimerList**：单例模式，使用**最小堆**管理所有定时器
- **EventLoop 集成**：每次 `loop()` 迭代前计算最小堆顶的超时时间作为 `poll()` 的超时参数，实现精确的定时触发

**定时器工作流程：**

```cpp
loop():
  1. timeout = timer_list_->ExpireMicroSeconds()  // 距下次超时的微秒数
  2. poller_->poll(timeout, ...)                  // 带超时的阻塞等待
  3. doTimerCheck() → timer_list_->CheckTimerExpired() // 检查并触发到期定时器
```

#### 🔟 Logger — 日志系统

基于 **Google Glog** 封装，提供简洁的宏接口：

```cpp
LOG_INFO("service started, listening on " << port)
LOG_ERROR("accept failed, errno=" << errno)
LOG_TRACE("connection state: " << state)
```

支持日志目录、自动清理、单文件大小限制等配置。

---

### 技术创新

#### ⚡ 1. eventfd 跨线程唤醒机制

传统的 Reactor 跨线程任务投递需要 `pipe()` 创建管道进行唤醒，而本项目采用 Linux 特有的 **`eventfd`** 替代 `pipe`：

- **更轻量**：`eventfd` 只是一个内核计数器，无需维护管道缓冲区
- **更高效**：只需一次 8 字节的 `write()`/`read()` 操作
- **更简洁**：无需创建两个 fd，无需关闭读写端

```cpp
static int CreateEventFd() {
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    // ...
}
```

#### 📦 2. 双 iovec scatter-gather 读取

`Buffer::readFd()` 采用 `readv()` 分散读，配合两个 `iovec`：

- **第一个 iovec**：指向 `Buffer` 内的可写区域
- **第二个 iovec**：指向栈上的 `extrabuf`（临时缓冲区）

当数据量超过 `Buffer` 剩余空间时，`readv()` 自动将溢出部分写入 `extrabuf`，再从 `extrabuf` 追加到 `Buffer` 中。这种方式**避免了预判数据量大小时的空间浪费**。

#### 🔄 3. 事件循环状态机

`EventLoop` 内部维护了精细的循环状态枚举：

```cpp
enum LoopState {
    kIdle,              // 空闲
    kPollerWaiting,     // 等待 I/O 事件
    kEventHandling,     // 处理 I/O 事件
    kTimerChecking,     // 检测定时器
    kPendingFunctorsCalling  // 执行跨线程任务
};
```

在 `queueInLoop()` 中，如果当前线程已经在执行 `kPendingFunctorsCalling` 状态，则即使调用方是本线程，也需要唤醒，确保新提交的任务在本轮循环中被执行。

#### 🧵 4. 双 Poller 后端可切换

通过编译宏 `_POLL` 可在 `epoll` 和 `poll` 之间切换：

```cmake
#add_definitions(-D_POLL)  # 取消注释启用 poll 后端
```

这一设计不仅增强了跨平台兼容性（某些类 Unix 系统不支持 epoll），也体现了策略模式的灵活运用。

#### 📐 5. 最小堆定时器

`TimerList` 使用**最小堆**（`std::push_heap` / `std::pop_heap` / `std::sort_heap`）管理定时器：

- 堆顶永远是最快要超时的定时器
- `ExpireMicroSeconds()` 直接返回堆顶的超时差值
- `CheckTimerExpired()` 只需从堆顶依次判断，遇到未超时的即可停止
- 增删操作时间复杂度为 $O(\log n)$

---

### 核心设计

#### 设计模式一览

| 模式 | 应用位置 | 说明 |
|------|---------|------|
| **Reactor** | 全局架构 | 事件驱动 + I/O 多路复用 |
| **One Loop Per Thread** | EventLoop + EventLoopThread | 每个 I/O 线程一个事件循环 |
| **Singleton（单例）** | `CSingleton<TcpServer>` / `TimerList` | 全局唯一服务实例和定时器列表 |
| **Strategy（策略模式）** | `Poller` → `EpollPoller` / `PollPoller` | 可切换的 I/O 后端 |
| **Template Method（模板方法）** | `PollerObject` 事件回调接口 | 定义事件处理骨架，子类实现细节 |
| **Proxy（代理模式）** | `HandlerProxyBasic` | 网络层与业务层解耦 |
| **Noncopyable（不可拷贝）** | `noncopyable` 基类 | 防止资源管理类被误拷贝 |
| **Factory（工厂方法）** | `Poller::newDefaultPoller()` | 创建默认 Poller 实例 |

#### 线程安全设计

1. **所有 I/O 操作必须在所属 EventLoop 线程执行**：`PollerObject` 的事件增删改查通过 `runInLoop()` 保证线程安全
2. **跨线程任务投递**：通过 `pendingFunctors_` + `eventfd` 唤醒机制实现
3. **锁的粒度控制**：`doPendingFunctors()` 使用 `swap` 技巧，仅在交换时持有锁，执行任务时不加锁
4. **原子变量**：`looping_` 使用 `std::atomic<bool>` 确保退出标志的可见性
5. **线程安全的单例**：C++11 静态局部变量初始化保证线程安全（`CSingleton`）

#### 关键路径的数据流

```
客户端发送数据
    │
    ▼
内核 Socket 接收缓冲区
    │
    ▼ (epoll_wait 触发可读事件)
EpollPoller::poll()
    │
    ▼
EventLoop::loop()  →  ProcessPollerEvents()  →  OnInputNotify()
    │
    ▼
TcpSocketHandler::OnInputNotify()
    │
    ├── inputBuffer_.readFd(fd_)     // readv → 用户态缓冲区
    │
    └── PacketStreamParser::parse_packet_length()
            │
            ├── return -1  →  OnCloseNotify()
            ├── return 0   →  break (继续接收)
            └── return len →  get_packet() → handler_proxy_->OnPacketComplete()
```

---

### 协议设计

本项目定义了简洁的二进制应用层协议：

| 字段 | 大小 | 说明 |
|------|------|------|
| `packet_len` | 4 字节 | 数据包长度（网络字节序），表示 magic_code + body 的总长度 |
| `magic_code` | 2 字节 | 魔数，固定为 `"XX"` (0x58, 0x58)，用于校验 |
| `body` | 可变 | 业务数据主体 |

**序列化示例：**

```cpp
// 发送端
std::string packet = PacketStreamParser::serialize_packet(body_data);
socket_handler->send(packet);

// 接收端（在 OnPacketComplete 回调中处理）
void OnPacketComplete(TcpSocketHandler *handler, std::string &data) {
    // data 即为 body 数据
    process(data);
}
```

---

### 构建与使用

#### 环境要求

- **操作系统**：Linux（需要 epoll 支持）
- **编译器**：GCC 9+（支持 C++17/20）
- **构建工具**：Make 或 CMake 3.10+
- **依赖**：Google Glog（日志）

#### 使用 Make 构建

```bash
cd core

# 编译动态库 (.so)
make

# 编译静态库 (.a)
make static

# 启用 poll 后端（默认 epoll）
# 在 Makefile 中取消注释 CXXFLAGS 中的 -D_POLL，或：
# 或者在 CMakeLists.txt 中取消 add_definitions(-D_POLL)

# 清理
make clean
```

#### 使用 CMake 构建

```bash
cd core
mkdir -p build && cd build
cmake ..
make
```

#### 快速开始

```cpp
#include "TcpServer.h"
#include "EventLoop.h"
#include "TcpSocketHandler.h"

// 1. 实现业务处理器
class MyHandler : public HandlerProxyBasic {
    void OnConnected(TcpSocketHandler *handler) override {
        LOG_INFO("new client connected");
    }
    void OnPacketComplete(TcpSocketHandler *handler, std::string &data) override {
        LOG_INFO("received packet, size=" << data.size());
        // 处理业务...
        handler->send("response data");
    }
    void OnClose(TcpSocketHandler *handler) override {
        LOG_INFO("client disconnected");
    }
};

int main(int argc, char *argv[]) {
    // 2. 初始化日志
    logger::InitLog(argv[0], {"./logs", 7, 100});

    // 3. 创建事件循环
    EventLoop loop;
    loop.setThreadID(std::this_thread::get_id());

    // 4. 绑定全局定时器列表
    TimerList timerList;
    loop.bindTimerList(&timerList);

    // 5. 创建服务端
    InetAddr addr("0.0.0.0", 12345);
    TcpServer server(&loop, addr);
    server.setThreadNum(4);             // 4 个工作线程
    server.bindHandlerProxy(new MyHandler());
    server.start();

    // 6. 启动事件循环
    loop.loop();
    return 0;
}
```

---

### 依赖

- **[Google Glog](https://github.com/google/glog)** — 高性能日志库（`3rdparty/google/glog/`）

---

### 项目结构

```
CReactorServer/
├── 3rdparty/                # 第三方依赖
│   └── google/glog/         # Google Glog 头文件
├── core/                    # 核心库
│   ├── include/             # 头文件
│   │   ├── Buffer.h         # 用户态缓冲区
│   │   ├── EpollPoller.h    # epoll I/O 多路复用
│   │   ├── EventLoop.h      # 事件循环
│   │   ├── EventLoopThread.h        # 事件循环线程
│   │   ├── EventLoopThreadPool.h    # 线程池
│   │   ├── logger.h         # 日志封装 (Glog)
│   │   ├── noncopyable.h    # 不可拷贝基类
│   │   ├── PacketStreamParser.h     # 数据包解析器
│   │   ├── Poller.h         # Poller 抽象基类
│   │   ├── PollerObject.h   # 事件驱动对象基类
│   │   ├── PollPoller.h     # poll I/O 多路复用
│   │   ├── singleton.h      # 线程安全单例模板
│   │   ├── Socket.h         # Socket 工具函数
│   │   ├── TcpServer.h      # TCP 服务端
│   │   ├── TcpSocketHandler.h       # TCP 连接处理器
│   │   ├── Timer.h          # 定时器
│   │   └── TimerList.h      # 定时器列表（最小堆）
│   ├── src/                 # 源文件
│   ├── CMakeLists.txt       # CMake 构建配置
│   └── Makefile             # Make 构建配置
└── libs/                    # 编译产物输出目录
```

---

### 许可证

本项目基于 MIT 许可证开源。
