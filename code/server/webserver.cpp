#include "webserver.h"
#include "epoller.h"
#include <cassert>
#include <cstring>
#include <sys/epoll.h>
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