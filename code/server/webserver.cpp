#include "webserver.h"
#include "epoller.h"
#include <asm-generic/socket.h>
#include <cassert>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <functional>
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
        timer_->add(fd, timeout_ms_, std::bind(&WebServer::close_conn, this, &users_[fd]));
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

void WebServer::deal_read(HttpConn *client) {
    assert(client != nullptr);
    extent_time(client);
    thread_pool_->AddTask(std::bind(&WebServer::on_read, this, client));
}

void WebServer::deal_write(HttpConn *client) {
    assert(client != nullptr);
    extent_time(client);
    thread_pool_->AddTask(std::bind(&WebServer::on_write, this, client));
}

void WebServer::on_read(HttpConn *client) {
    assert(client);
    int ret = -1;
    int read_errno = 0;
    ret = client->read(&read_errno);
    if (ret <= 0 && read_errno != EAGAIN) {
        close_conn(client);
        return;
    }
    on_process(client);
}

void WebServer::on_write(HttpConn *client) {
    assert(client != nullptr);
    int ret = -1;
    int write_errno = 0;
    ret = client->write(&write_errno);
    if (client->to_write_bytes() == 0) {
        if (client->is_keep_alive()) {
            on_process(client);
            return;
        }
    } else if (ret < 0) {
        if (write_errno == EAGAIN) {
            epoller_->mod_fd(client->get_fd(), conn_event_ != EPOLLOUT);
            return;
        }
    }
    close_conn(client);
}

void WebServer::on_process(HttpConn *client) {
    if (client->process()) {
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLOUT); 
    } else {
        epoller_->mod_fd(client->get_fd(), conn_event_ | EPOLLIN);
    }
}

void WebServer::start() {
    int time_ms = -1;
    if (!is_close_) {
        LOG_INFO("======== Server Start ========");
    }
    while (!is_close_) {
        if (timeout_ms_ > 0) {
            time_ms = timer_->GetNextTick();
        }
        int event_cnt = epoller_->wait(time_ms);
        for (int i = 0; i < event_cnt; i++) {
            int fd = epoller_->get_event_fd(i);
            uint32_t events = epoller_->get_events(i);
            if (fd == listen_fd_) {
                deal_listen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                close_conn(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                deal_read(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                deal_write(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected Event!");
            }
        }
    }
}

bool WebServer::init_socket() {
    int ret;
    struct sockaddr_in addr;
    if (port_ > 65535 || port_ < 1024) {
        LOG_ERROR("Port:%d erorr!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);
    struct linger opt_linger = {0};
    if (open_linger_) {
        opt_linger.l_onoff = 1;
        opt_linger.l_linger = 1;
    }

    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);

    if (listen_fd_ < 0) {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_LINGER, &opt_linger, sizeof(opt_linger));
    if (ret == -1) {
        close(listen_fd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    int optval = 1;

    ret = setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval, sizeof(int));
    if (ret == -1) {
        close(listen_fd_);
        LOG_ERROR("set socket setsockopt erorr!", port_);
        return false;
    }

    ret = bind(listen_fd_, (struct sockaddr *) &addr, sizeof(addr));
    if (ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Bind Port:%d error!", port_);
        return false;
    }

    ret = listen(listen_fd_, 6);
    if (ret < 0) {
        close(listen_fd_);
        LOG_ERROR("Listen Port:%d error!", port_);
        return false;
    }

    ret = epoller_->add_fd(listen_fd_, listen_event_ | EPOLLIN);
    if (ret == 0) {
        close(listen_fd_);
        LOG_ERROR("Add listen error");
        return false;
    }
    set_fd_nonblock(listen_fd_);
    LOG_INFO("Server Port:%d", port_);
    return true;
}

int WebServer::set_fd_nonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void WebServer::extent_time(HttpConn *client) {
    assert(client != nullptr);
    if (timeout_ms_ > 0) {
        timer_->adjust(client->get_fd(), timeout_ms_);
    }
}

void WebServer::init_event_mode(int trig_mode) {
    listen_event_ = EPOLLRDHUP;
    conn_event_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trig_mode) {
        case 0:
            break;
        case 1:
            conn_event_ |= EPOLLET;
            break;
        case 2:
            listen_event_ |= EPOLLET;
            break;
        case 3:
            listen_event_ |= EPOLLET;
            conn_event_ |= EPOLLET;
            break;
        default:
            listen_event_ |= EPOLLET;
            conn_event_ |= EPOLLET;
            break;
    }
    HttpConn::is_ET = (conn_event_ & EPOLLET);
}