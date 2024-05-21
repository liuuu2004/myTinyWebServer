#include "buffer.h"
#include <algorithm>
#include <bits/types/struct_iovec.h>
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <strings.h>
#include <sys/uio.h>


Buffer::Buffer(int init_buffer_size) : buffer_(init_buffer_size), read_pos_(0), write_pos_(0) {};

size_t Buffer::ReadableBytes() const {
    return write_pos_ - read_pos_;
}

size_t Buffer::WritableBytes() const {
    return buffer_.size() - write_pos_;
}

size_t Buffer::PrependableBytes() const {
    return read_pos_;
}

const char * Buffer::Peek() const {
    return BeginPtr() + read_pos_;
}

void Buffer::Retrieve(size_t len) {
    assert(len <= ReadableBytes());
    read_pos_ += len;
}

void Buffer::RetrieveUntil(const char *end) {
    assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    memset(&buffer_[0], 0, buffer_.size());
    read_pos_ = 0;
    write_pos_ = 0;
}

std::string Buffer::RetrieveAllToString() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char * Buffer::BeginWriteConst() const {
    return BeginPtr() + write_pos_;
}

char * Buffer::BeginWrite() {
    return BeginPtr() + write_pos_;
}

void Buffer::HasWritten(size_t len) {
    write_pos_ += len;
}

void Buffer::Append(const std::string &str) {
    Append(str.data(), str.length());
}

void Buffer::Append(const void *data, size_t len) {
    assert(data != nullptr);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const char *str, size_t len) {
    assert(str != nullptr);
    EnsureWritable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}

void Buffer::Append(const Buffer &buffer) {
    Append(buffer.Peek(), buffer.ReadableBytes());
}

void Buffer::EnsureWritable(size_t len) {
    if (WritableBytes() < len) {
        MakeSapce(len);
    }
    assert(WritableBytes() >= len);
}

ssize_t Buffer::ReadFd(int fd, int *save_errno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableBytes();

    iov[0].iov_base = BeginPtr() + write_pos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const size_t len = readv(fd, iov, 2);

    if (len < 0) {
        *save_errno = errno;
    } else if (static_cast<size_t>(len) <= writable) {
        write_pos_ += len;
    } else {
        write_pos_ = buffer_.size();
        Append(buff, len - writable);
    }
}

ssize_t Buffer::WriteFd(int fd, int *save_errno) {
    size_t read_size = ReadableBytes();
    ssize_t len = write(fd, Peek(), read_size);
    if (len < 0) {
        *save_errno = errno;
        return len;
    }
    read_pos_ += len;
    return len;
}

char * Buffer::BeginPtr() {
    return &*buffer_.begin();
}

const char * Buffer::BeginPtr() const {
    return &*buffer_.begin();
}

void Buffer::MakeSapce(size_t len) {
    if (WritableBytes() + PrependableBytes() < len) {
        buffer_.resize(write_pos_ + len + 1);
    } else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr() + read_pos_, BeginPtr() + write_pos_, BeginPtr());
        read_pos_ = 0;
        write_pos_ = read_pos_ + readable;
        assert(readable == ReadableBytes());
    }
}