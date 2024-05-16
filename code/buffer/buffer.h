#ifndef BUFFER_H
#define BUFFER_H

#include <atomic>
#include <cstddef>
#include <string>
#include <sys/types.h>
#include <vector>


class Buffer {
public:
    Buffer(int init_buffer_size = 1024);

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char *Peek() const;
    void EnsureWritable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);

    void RetrieveAll();
    std::string RetrieveAllToString();

    const char *BeginWriteConst() const;
    char *BeginWrite();

    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buffer);

    ssize_t ReadFd(int fd, int *errno);
    ssize_t WriteFd(int fd, int *errno);

private:
    char * BeginPtr();
    const char * BeginPtr() const;
    
    std::vector<char> buffer_;
    std::atomic<std::size_t> read_pos_;
    std::atomic<std::size_t> write_pos_;
};


#endif