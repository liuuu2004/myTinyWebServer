#include "httpconn.h"
#include <cassert>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <netinet/in.h>
#include <sys/types.h>

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