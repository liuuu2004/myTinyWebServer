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

    /**
     * start the event-loop and waits for every using epoll_wait and handles them accordingly
    */
    void start();
    
private:
    /**
     * initialize the lsitening socket and configures socket options binds the socket to
     * the specified port and starts listening for incoming connections. adds the listening socket
     * to the epoll instance and sets it to non-blocking mode
     * @return true if init successed, otherwise false
    */
    bool init_socket();

    /**
     * configures the event handling mode for the server based on the provided trig_mode
     * @param trig_mode determins how events are detected and procesed by the epoll instance
    */
    void init_event_mode(int trig_mode);

    /**
     * add a new client connexction and set it up with the epoll instance
     * @param fd file descriptor to be added
     * @param addr client address
    */
    void add_client(int fd, sockaddr_in addr);

    /**
     * accepts nre connections and adds them as clients
    */
    void deal_listen();
    
    /**
     * handles write events similarly to read events
     * @param client client connection to be dealt with
    */
    void deal_write(HttpConn *client);

    /**
     * handles read events by extending the connection's timeout and adding a read task
     * to the thread pool
     * @param client client connection to be dealt with
    */
    void deal_read(HttpConn *client);

    /**
     * send an error message to the client and close the connection
     * @param fd file describer of the client
     * @param info information to be sent
    */
    void send_error(int fd, const char *info);

    /**
     * extend the timeout duration for a client's connection
     * @param client client connection whose timeout duration is to be extended
    */
    void extent_time(HttpConn *client);

    /**
     * close a client connection and removes it from the epoll instance
     * @param client client connection to be closed
    */
    void close_conn(HttpConn *client);

    /**
     * reads data from the client and process it
     * @param client client connection to read data from
    */
    void on_read(HttpConn *client);

    /**
     * write data to the client and handle any errors
     * @param client client connection to write data to
    */
    void on_write(HttpConn *client);

    /**
     * process the client's request and updates the epoll instance accordingly
     * @param client client connection to be processed
    */
    void on_process(HttpConn *client);

    static const int MAX_FD_ = 65535;

    /**
     * sets a file descriptor to non-blocking mode
     * @param fd file descriptor to be set
     * @return whether successed
    */
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