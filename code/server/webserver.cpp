#include "webserver.h"
#include "epoller.h"
#include <cassert>
#include <cstring>
#include <iterator>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

WebServer::WebServer(int port, int trig_mode, int timeout_ms, bool opt_linger, int sql_port,
              const char *sql_user, const char *sql_pwd, const char *db_name,
              int conn_pool_num, int thread_num, bool open_log, int log_level, int log_que_size) :
    port_(port), open_linger_(opt_linger), timeout_ms_(timeout_ms), is_close_(false),
    timer_(new HeapTimer()), thread_pool_(new ThreadPool(thread_num)), epoller_(new Epoller()) {
    src_dir_ = getcwd(nullptr, 256);
    assert(src_dir_);
    strncat(src_dir_, "/resources/", 16);
    HttpConn::user_cnt = 0;
    HttpConn::src_dir = src_dir_;
    SqlConnPool::instance()->init("localhost", sql_port, sql_user, sql_pwd, db_name);

    init_event_mode(trig_mode);
    if (!init_socket()) {
        is_close_ = true;
    }

    if (open_log) {
        Log::instance()->init(log_level, "./log", ".log", log_que_size);
        if (is_close_) {
            LOG_ERROR("======== Server Init Error! ========");
        } else {
            LOG_INFO("======== Server Init ========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, opt_linger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                (listen_event_ & EPOLLET ? "ET" : "LT"),
                (conn_event_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("srcDir: %s", HttpConn::src_dir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", conn_pool_num, thread_num);
        }
    }
}

WebServer::~WebServer() {
    close(listen_fd_);
    is_close_ = true;
    free(src_dir_);
    SqlConnPool::instance()->close_pool();
}

void WebServer::send_error(int fd, const char *info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client [%d] error!", fd);
    }
    close(fd);
}

void WebServer::close_conn(HttpConn *client) {
    assert(client != nullptr);
    LOG_INFO("Client[%d] quit!", client->get_fd());
    epoller_->del_fd(client->get_fd());
    client->close();
}

void WebServer::add_client(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeout_ms_ > 0) {
        timer_->add(fd, timeout_ms_, std::bind(&::WebServer::close_conn, this, &users_[fd]));
    }
    epoller_->add_fd(fd, EPOLLIN | conn_event_);
    set_fd_nonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].get_fd());
}

void WebServer::deal_listen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do {
        int fd = accept(listen_fd_, (struct sockaddr *) &addr, &len);
        if (fd < 0) {
            return;
        } else if (HttpConn::user_cnt >= MAX_FD_) {
            send_error(fd, "server busy");
            LOG_WARN("Clients are full");
            return;
        }
        add_client(fd, addr);
    } while (listen_event_ & EPOLLET);
}