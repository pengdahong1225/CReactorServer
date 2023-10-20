//
// Created by Messi on 2023/6/9.
//

#include "Buffer.h"
#include <sys/types.h>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <sys/uio.h>

using namespace core;
using namespace core::net;

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

Buffer::Buffer()
        : buffer_(kCheapPrepend + kInitialSize),
          readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend)
{
    assert(readableBytes() == 0);
    assert(writeableBytes() == kInitialSize);
    assert(prependableBytes() == kCheapPrepend);
}

size_t Buffer::readableBytes()
{
    return writerIndex_ - readerIndex_;
}

size_t Buffer::writeableBytes()
{
    return buffer_.size() - writerIndex_;
}

size_t Buffer::prependableBytes()
{
    return readerIndex_;
}

ssize_t Buffer::readFd(int fd)
{
    char extrabuf[kInitialSize]; // 临时栈
    memset(extrabuf, 0, sizeof extrabuf);
    struct iovec vec[2];
    const size_t writeable = writeableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    const int iovcnt = (writeable < sizeof extrabuf) ? 2 : 1; // 按照顺序 -- 填充缓冲区
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
        return -1;
    else if (static_cast<size_t>(n) <= writeable)
        writerIndex_ += n;
    else {
        writerIndex_ += n;
        append(extrabuf, n - writeable); // 追加
    }
    return n;
}

void Buffer::append(const char *data, size_t len)
{
    ensureWriteableBytes(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::ensureWriteableBytes(size_t len)
{
    if (writeableBytes() < len)
        makeSpace(len);
    assert(writeableBytes() >= len);
}

void Buffer::hasWritten(size_t len)
{
    assert(len <= writeableBytes());
    writerIndex_ += len;
}

void Buffer::makeSpace(size_t len)
{
    if (writeableBytes() + prependableBytes() < len + kCheapPrepend) {
        // 扩容
        buffer_.resize(writerIndex_ + len);
    } else {
        // 内部腾挪
        assert(kCheapPrepend < readerIndex_);
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_,
                  begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;
        writerIndex_ = readerIndex_ + readable;
        assert(readable == readableBytes());
    }
}

const char *Buffer::peek() const
{
    return begin() + readerIndex_;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    if (len < readableBytes())
        readerIndex_ += len;
    else
        retrieveAll();
}

void Buffer::retrieveAll()
{
    readerIndex_ = kCheapPrepend;
    writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString()
{
    return retrieveAsString(readableBytes());
}

std::string Buffer::retrieveAsString(size_t len)
{
    assert(len <= readableBytes());
    std::string result(peek(), len);
    retrieve(len);
    return result;
}


