#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <cstdint>
#include <memory>
#include <unordered_map>

#include "../timer/heaptimer.h"
#include "../pool/threadpool.h"
#include "epoller.h"
#include "../http/httpconn.h"

class WebServer {
public:

private:

private:
    int port_;
    bool open_linger_;
    int timeout_ms_;
    bool is_close_;
    int listen_fd_;
    char *src_dir_;

    uint32_t listen_event_;
    uint32_t conn_event_;

    std::unique_ptr<HeapTimer> timer_;
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif