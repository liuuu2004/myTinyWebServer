#include "httpconn.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

HttpConn::HttpConn() {
    fd_ = -1;
    addr_ = {0};
    is_close_ = true;
};

HttpConn::~HttpConn() {
    close();
}

int HttpConn::get_fd() const {
    return fd_;
}

sockaddr_in HttpConn::get_addr() const {
    return addr_;
}

int HttpConn::get_port() const {
    return addr_.sin_port;
}

const char * HttpConn::get_ip() const {
    return inet_ntoa(addr_.sin_addr);
}

bool HttpConn::is_keep_alive() const {
    return request_.is_keep_alive();
}

int HttpConn::to_write_bytes() const {
    return iov_[0].iov_len + iov_[1].iov_len;
}

void HttpConn::init(int sock_fd, const sockaddr_in &addr) {
    assert(sock_fd > 0);
    user_cnt++;
    addr_ = addr;
    fd_ = sock_fd;
    write_buffer_.RetrieveAll();
    read_buffer_.RetrieveAll();
    is_close_ = false;
    LOG_INFO("Client[%d](%s:%s) in, userCount:%d", fd_, get_ip(), get_port(), (int) user_cnt);
}

ssize_t HttpConn::read(int *save_error) {
    ssize_t len = -1;
    do {
        len = read_buffer_.ReadFd(fd_, save_error);
        if (len <= 0) {
            break;
        }
    } while (is_ET);
    return len;
}

ssize_t HttpConn::write(int *save_error) {
    ssize_t len = -1;
    do {
        len = writev(fd_, iov_, iov_cnt_);
        if (len <= 0) {
            *save_error = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov_[0].iov_len) {
            iov_[1].iov_base = (uint8_t *) iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len != 0) {
                write_buffer_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        } else {
            iov_[0].iov_base = (uint8_t *) iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            write_buffer_.RetrieveAll();
        }
    } while (is_ET || to_write_bytes() > 10240);
    return len;
}

void HttpConn::close() {
    response_.unmap_file();
    if (is_close_ == false) {
        is_close_ = true;
        user_cnt--;
        ::close(fd_);
        LOG_INFO("Client[%d](%s:%s) quit, UserCount:%d", fd_, get_ip(), get_port(), (int) user_cnt);
    }
}

bool HttpConn::process() {
    // initialize the request object to reset its state for a new request
    request_.init();

    // check if there are any readable bytes in the read buffer
    if (read_buffer_.ReadableBytes() <= 0) {
        // if no readable bytes, return false indicating no request to process
        return false;
    } else if (request_.parse(read_buffer_)) {
        // if the request is successfully parsed, log the request path
        LOG_DEBUG("%s", request_.path().c_str());

        /**
         * initialize the response object with the source directory, retuest path,
         * keep-alive flag and a 200 OK status code
        */
        response_.init(src_dir, request_.path(), request_.is_keep_alive(), 200);
    } else {
        /** if the request parsing failed, initialize the response object with a 400
         *  bad request status code
        */
        response_.init(src_dir, request_.path(), false, 400);
    }

    // generate the HTTP response and store it in the write buffer
    response_.make_response(write_buffer_);

    // set up the iovec structure for the response headers
    iov_[0].iov_base = const_cast<char *>(write_buffer_.Peek());
    iov_[0].iov_len = write_buffer_.ReadableBytes();
    iov_cnt_ = 1;

    // check if there is a file to be sent as part of the response
    if (response_.file_len() > 0 && response_.file()) {
        // if there is a file, set up the iovec structure for the file
        iov_[1].iov_base = response_.file();
        iov_[1].iov_len = response_.file_len();
        iov_cnt_ = 2;
    }

    // log the file size, the number of iovec structures, and the total bytes to be written
    LOG_DEBUG("filesize:%d, %d  to %d", response_.file_len(), iov_cnt_, to_write_bytes());

    return true;
}