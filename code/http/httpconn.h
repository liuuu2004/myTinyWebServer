/**
 * 
*/


#ifndef HTTP_CONN_H_
#define HTTP_CONN_H_

#include <atomic>
#include <bits/types/struct_iovec.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "httprequest.h"
#include "netinet/in.h"
#include "httpresponse.h"
#include "../buffer/buffer.h"

class HttpConn {
public:
    HttpConn();
    ~HttpConn();
    
    /**
     * init variables and clear containers
     * @param sock_fd socket file descriptor
     * @param addr address of TODO
    */
    void init(int sock_fd, const sockaddr_in &addr);

    /**
     * read data from socket into the read_buffer_ buffer
     * @param save_error pointer to an integer where the function stores the error number
     *                   if an error occurs
    */
    ssize_t read(int *save_error);

    /**
     * write data from the write_buffer_ buffer to the socket
     * @param save_error pointer to an integer where the function stores the error number
     *                   if an error occurs
    */
    ssize_t write(int *save_error);

    /**
     * close connection and free resources
    */
    void close();

    /**
     * get the file descriptor
     * @return file dexcriptor
    */
    int get_fd() const;

    /**
     * get the port of the connection
     * @return port #
    */
    int get_port() const;

    /**
     * get the ip of the TODO
     * @return ip of TODO
    */
    const char *get_ip() const;

    /**
     * get the address of the TODO
     * @return address of the socket TODO
    */
    sockaddr_in get_addr() const;

    /**
     * managing the parsing of an HTTP request and thepreparation of the corresponding HTTP
     * response. It handles reading from the input buffer, parsing the retuest, generating the
     * response, and setting up the IO vectors for effecient writing. The function ensures that
     * the resposne is properly formatted and ready to be sent back to the client.
     * @return true if the resposne is ready to be sent
    */
    bool process();

    /**
     * return the total number of bytes that are pending to be written to the sockert
     * @return the total number of bytes that are pending to be written to the sockert
    */
    int to_write_bytes() const;
    
    /**
     * check whether the request should be kept alive
     * @return whether the request should be kept alive
    */
    bool is_keep_alive() const;

    static bool is_ET;
    static const char *src_dir;
    static std::atomic<int> user_cnt;

private:
    int fd_;
    struct sockaddr_in addr_;

    bool is_close_;

    int iov_cnt_;
    struct iovec iov_[2];

    Buffer read_buffer_;
    Buffer write_buffer_;

    HttpRequest request_;
    HttpResponse response_;

};


#endif