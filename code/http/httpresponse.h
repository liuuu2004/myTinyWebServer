/**
 * An HTTP response is the message a server sends back to a client afterreceiving and processing
 * an HTTP request. The structure of an HTTP response is also standardized and consists of
 * several parts (three parts here: status line, headers and body), each serving a specific
 * function.
 *   1. Status Line:
 *       HTTP Version: specifies the version of the HTTP protocal being used, such as 'HTTP/1.1'
 *       Status Code: A three digit number that indicates the result of the request. 
 *
 *       Example: HTTP/1.1 200 OK
 *
 *   2. Headers:
 *       Content-Type: indicates the media type of the resource sent to the client, such as
 *                     'text/html', 'application/json', or 'image/jpeg'
 *       Content-Length: indicates the size of the response body in bytes
 *       Connection: whether or not the connection should be kept alive after this response
 *  
 *       Example: Content-type: text/html
 *                Content-length: 128
 *                Connection: keep-alive
 *
 *   3. Body: the actual content of the response to be sent to the client. This could be an HTML
 *            page, JSON data, an image, or any other type of data, depending on the request and
 *            response headers.
 *       Example:    <html>
 *                   <head>
 *                       <title>Example Page</title>
 *                   </head>
 *                   <body>
 *                       <hi>Hello, World!</hi>
 *                   </body>
 *                   </heml>
 * 
 *   4. Example of a Full HTTP Response:
 *       +-------------+-----------------+-------------------+
 *       | Status Line | Headers         | Body              |
 *       +-------------+-----------------+-------------------+
 *       | HTTP/1.1 200 OK               | Content of file   |
 *       | Content-Type: text/html       | or error message  |
 *       | Content-Length: 123           |                   |
 *       | Connection: keep-alive        |                   |
 *       +-------------------------------+-------------------+
 *
 *       
*/


#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <string>
#include <fcntl.h>
#include <unordered_map>
#include <sys/mman.h>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();
    /**
     * init variables with specific value
     * @param src_dir diractor to store resources
     * @param path path to store resource
     * @param is_keep_alive shall we keep the connction alive for further requests
     * @param code HTTP reaponse code
    */
    void init(const std::string &src_dir, std::string &path, bool is_keep_alive = false, int code = -1);

    /**
     * create a HTTP response based on the requested resource and its status
     * @param buffer store the HTTP response
    */
    void make_response(Buffer &buffer);

    /**
     * unmap file
    */
    void unmap_file();

    /**
     * get the mm file
     * @return mm file
    */
    char *file();

    /**
     * get the length of the file
     * @return length of the file
    */
    size_t file_len() const;

    /**
     * generate an HTML error page and append it to the response buffer
     * @param buffer a buffer to store error content
     * @param message message to be stored
    */
    void error_content(Buffer &buffer, std::string message);

    /**
     * get the status code
     * @return status code
    */
    int code() const;

private:  // methods
    /**
     * add the HTTP state line to the buffer, including the HTTP version, status
     * code and status message
     * @param buffer store the formation in the buffer
    */
    void add_state_line(Buffer &buffer);

    /**
     * add the header information to the buffer, including the connection type
     * and content type
     * @param buffer store the information in the buffer
    */
    void add_header(Buffer &buffer);

    /**
     * add the content of the requested file to the buffer by memory-mapping in the file
     * @param buffer store the information in the buffer
    */
    void add_content(Buffer &buffer);

    /**
     * relocate to error html
    */
    void error_html();

    /**
     * get the type of a specific file
     * @return the type of a specific file
    */
    std::string get_file_type();

private:  // variables
    /**
     * store HTTP response code
    */
    int code_;

    /**
     * indicates whether the connection should be kept alive. if true, the connection is
     * kept for further requests
    */
    bool is_keep_alive_;

    /**
     * store the path to the requested resource or file
    */
    std::string path_;

    /**
     * stores the source dictionary path where the requested reesources or files are located
    */
    std::string src_dir_;

    /**
     * pointer to the memory-mapped file. used to access file content efficiently
    */
    char *mm_file_;

    /**
     * stores the metadatta(e.g. size, permissions) of the memory-mapped file
    */
    struct stat mm_file_stat_;

    /**
     * maps file exxtensions to their corresponding MIME types(e.g. html -> test/heml)
    */
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE_;

    /**
     * maps HTTP status codes to their corresponding status maeesges(e.g. 200 -> OK)
    */
    static const std::unordered_map<int, std::string> CODE_STATUS_;

    /**
     * maps error status codes to their corresponding error HTML paths(e.g. 404 -> /40.html)
    */
    static const std::unordered_map<int, std::string> CODE_PATH_;
};

#endif