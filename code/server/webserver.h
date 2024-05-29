#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include <unordered_map>

#include "../timer/heaptimer.h"
#include "../pool/threadpool.h"
#include "epoller.h"
#include "../http/httpconn.h"

class WebServer {
public:
    WebServer(int port, int trig_mode, int timeout_ms, bool opt_linger, int sql_port,
              const char *sql_user, const char *sql_pwd, const char *db_name,
              int conn_pool_num, int thread_num, bool open_log, int log_level, int log_que_size);
    ~WebServer();
    void start();
    
private:
    bool init_socket();
    void init_event_mode(int trig_mode);
    void add_client(int fd, sockaddr_in addr);

    void deal_listen();
    void deal_write(HttpConn *client);
    void deal_read(HttpConn *client);

    /**
     * send an error message to the client and close the connection
     * @param fd file describer of the client
     * @param info information to be sent
    */
    void send_error(int fd, const char *info);
    void extent_time(HttpConn *client);
    void close_conn(HttpConn *client);

    void on_read(HttpConn *client);
    void on_write(HttpConn *client);
    void on_process(HttpConn *client);

    static const int MAX_FD_ = 65535;
    static int set_fd_nonblock(int fd);

private:
    /**
     * the port number on which the web server will listen for incoming connections
    */
    int port_;

    /**
     * a flag indicates whether the socket option SO_LINGER is enabled. this option 
     * determines the behavior of the socket when it's closed, specially whether it will
     * linger to send remaining data
    */
    bool open_linger_;

    /**
     * the timeout of waiting in ms
    */
    int timeout_ms_;

    /**
     * whether the server is currently closed
    */
    bool is_close_;

    /**
     * the file descriptor for the listening socket, the socket is used to accept
     * incoming connections
    */
    int listen_fd_;

    /**
     * a pointer to a character array holding the directory path for server resources,
     * this dictionary contains static files that may be served by the web server
    */
    char *src_dir_;

    /**
     * event configuration for the lsitening socket, this holds the events that the epoll
     * instance will monitor for the listening socket
    */
    uint32_t listen_event_;

    /**
     * event configuration for the client connection sockets, this holds the events
     * that the epoll instance will monitor for each client socket
    */
    uint32_t conn_event_;

    /**
     * manage time-based events, such as connection timeouts
    */
    std::unique_ptr<HeapTimer> timer_;

    /**
     * manage a collection of threads that handle client requests, allowing the server
     * to process mutiple requests concurrently
    */
    std::unique_ptr<ThreadPool> thread_pool_;

    /**
     * encapsulates the epoll functionally, managing the file descriptors and events
     * for the server
    */
    std::unique_ptr<Epoller> epoller_;

    /**
     * maps client socket file descriptors to HttpConn instances, stores the state and data
     * for each connected client, enabling the server to manage multiple client connections
     * efficiently
    */
    std::unordered_map<int, HttpConn> users_;
};

#endif