//
// Created by Messi on 2023/6/9.
//

#ifndef CREACTORSERVER_BUFFER_H
#define CREACTORSERVER_BUFFER_H

#include "noncopyable.h"
#include <vector>
#include <sys/types.h>
#include <string>

/*
 * 底层数据包缓冲区
 * 从尾部写入 从头部读出
 *
 * +-------------------+------------------+------------------+
 * | prependable bytes |  readable bytes  |  writable bytes  |
 * |                   |     (CONTENT)    |                  |
 * +-------------------+------------------+------------------+
 * |                   |                  |                  |
 * 0      <=      readerIndex   <=   writerIndex    <=     size
 */

namespace reactor {
    class Buffer {
    public:
        static const size_t kCheapPrepend = 8;
        static const size_t kInitialSize = 1024;

        Buffer();
        ~Buffer() = default;

        size_t readableBytes();
        size_t writeableBytes();
        size_t prependableBytes();
        ssize_t readFd(int fd);// 内核缓冲区 -> 用户缓冲区
        void append(const char *data, size_t len);
        void ensureWriteableBytes(size_t len);// 确保空间充足 不够就扩充
        void hasWritten(size_t len);
        const char *peek() const;
        void retrieve(size_t len);
        void retrieveAll();
        std::string retrieveAllAsString();
        std::string retrieveAsString(size_t len);

    private:
        void makeSpace(size_t len);
        // 返回缓冲区起始地址
        char *begin() {
            return &*buffer_.begin();
        }
        const char *begin() const {
            return &*buffer_.begin();
        }
        char *beginWrite() {
            return begin() + writerIndex_;
        }
        const char *beginWrite() const {
            return begin() + writerIndex_;
        }

    private:
        std::vector<char> buffer_;
        size_t readerIndex_;
        size_t writerIndex_;
    };
}

#endif //CREACTORSERVER_BUFFER_H+++
