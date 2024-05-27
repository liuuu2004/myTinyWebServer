#include "httpconn.h"
#include <netinet/in.h>

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