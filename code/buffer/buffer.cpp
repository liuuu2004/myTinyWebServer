#include "buffer.h"
#include <cassert>
#include <cstddef>
#include <strings.h>

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

void Buffer::RetrieveAll() {
    bzero(&buffer_[0], buffer_.size());
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

ssize_t Buffer::ReadFd(int fd, int *errno) {
    char buff[65535];
    
}