#ifndef BUFFER_H
#define BUFFER_H

#include <atomic>
#include <cstddef>
#include <string>
#include <sys/types.h>
#include <vector>
#include <sys/uio.h>
#include <unistd.h>

class Buffer {
public:
    /**
     * init a Buffer class whose read pos and write pos are 0,
     * default buffer size = 1024
     * @param init_buffer_size the initial size of the buffer
    */
    Buffer(int init_buffer_size = 1024);

    /**
     * get the number of bytes in the buffer that are writable
     * @return number of bytes in buffer that are writable
    */
    size_t WritableBytes() const;

    /**
     * get the number of bytes in the buffer that are readable
     * @return number of bytes in the buffer that are readable
    */
    size_t ReadableBytes() const;

    /**
     * get the current position of read pointer
     * @return curretn position of read pointer
    */
    size_t PrependableBytes() const;

    /**
     * get the address of the current read position
     * @return address of the current read position
    */
    const char *Peek() const;

    /**
     * ensure the number of writable bytes is more than
     * len. if not, it will call MakeSpace().
     * @param len the number of writable bytes should be
     *            greater than this
    */
    void EnsureWritable(size_t len);

    /**
     * this will be called after written something into the buffer.
     * @param len number of new bytes that are written
    */
    void HasWritten(size_t len);

    /**
     * move readable pos in the size of len
     * @param len size of readable bytes moved
    */
    void Retrieve(size_t len);

    /**
     * retrieve until meets character end
     * @param end retrieve until meets this
    */
    void RetrieveUntil(const char *end);

    /**
     * set everything in buffer to 0 and set read pos, write pos to 0
    */
    void RetrieveAll();

    /**
     * get everything readable in the buffer and set everything in buffer to 0
     * and set read pos, write pos to 0
     * @return the entire readable string in the buffer
    */
    std::string RetrieveAllToString();

    /**
     * obtain a pointer to the position in the buffer where bew data can be written
     * @return position in the buffer where new data can be written
    */
    const char *BeginWriteConst() const;

    /**
     * obtain a pointer to the position in the buffer where new data can be written
     * @return position in the buffer where new data can be written
    */
    char *BeginWrite();

    /**
     * append the entire array to the buffer
     * @param str bytes from which to be copied
    */
    void Append(const std::string &str);

    /**
     * append a specified number of bytes from a given character array to the buffer
     * @param str bytes from which to be copied
     * @param len number of bytes to be copied
    */
    void Append(const char *str, size_t len);

    /**
     * append a specified number of bytes from a given array of any type to the buffer
     * @param data bytes from which to be copied
     * @param len number of bytes to be copied
    */
    void Append(const void *data, size_t len);

    /**
     * append all readable bytes from a given buffer to the buffer
     * @param buffer bytes from which to be copied
    */
    void Append(const Buffer &buffer);

    /**
     * define a temporary buffer
     * @param fd the file descriptor from which data is to be read
     * @param errno a pointer to an integer where the error code will be saved 
     *              if any error occurs
     * @return the number of bytes actually read. return -1 if any error occurs
    */
    ssize_t ReadFd(int fd, int *save_errno);

    /**
     * @param fd the file descriptor from which data is to be written
     * @param errno a pointer to an integer where the error code will be saved
     *              if any error occurs
     * @return the number of bytes actually written. return -1 if any error occurs
    */
    ssize_t WriteFd(int fd, int *save_errno);

private:
    /**
     * get the begining pointer of the underlying buffer
     * @return the begining pointer of the underlying buffer
    */
    char * BeginPtr();

    /**
     * get the const begining pointer of the underlying buffer
     * @return the const begining pointer of the underlying buffer
    */
    const char * BeginPtr() const;

    /**
     * ensure that there is enough space in the buffer to write additional
     * data. if the curretn writable space plus any space at the begining is 
     * insufficient, the buffer will be resized. otherwise, the existing data
     * is moved to create enough space.
     * @param len the length of additional space needed in the buffer
    */
    void MakeSapce(size_t len);
    
    std::vector<char> buffer_;
    std::atomic<std::size_t> read_pos_;
    std::atomic<std::size_t> write_pos_;
};


#endif