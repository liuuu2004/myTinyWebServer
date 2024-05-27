/**
 * An HTTP Request is s message sent by a client to a server, requesting a resource or servce.
 * The structure of an HTTP request is standardized and consists of serveral parts, each serving
 * a specific purpose. (three parts here: status line, headers and body), each serving a specific
 * function.
 *   1. Status Line:
 *       HTTP method, requested resource (path) and HTTP version
 *
 *       Example:  Get /index.heml HTTP/1.1
 *
 *   2. Headers:
 *       Contain various metadata for the request.
 *
 *       Example:
 *         Host: www.example.com
 *         Connection: keep-alive
 *         Accept: text/html
 *         User-Agent: Mozilla/5.0
 *         Content-Length: 27
 *         Content-Type: application/x-www-form-urlencoded
 *
 *   3. Body:
 *       Contain the actual data sent with the request, typically for POST requests
 *
 *       Example:  username=JohnDoe&password=123456
 *
 *  Example of HTTP Response Message:
 *      +-------------+-----------------------------------+-------------------+
 *      | Request Line| Headers                           | Body              |
 *      +-------------+-----------------------------------+-------------------+
 *      | GET /index.html HTTP/1.1                        | User data         |
 *      | Host: www.example.com                           | or form data      |
 *      | Connection: keep-alive                          |                   |
 *      | Accept: text/html                               |                   |
 *      | User-Agent: Mozilla/5.0                         |                   |
 *      | Content-Length: 27                              |                   |
 *      | Content-Type: application/x-www-form-urlencoded |                   |
 *      +-------------------------------------------------+-------------------+
 *
 *
*/




#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>

#include "mysql/mysql.h"
#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"

class HttpRequest {
public:
    enum PARSE_STATE_ {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,
    };

    enum HTTP_CODE_ {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    HttpRequest() {
        init();
    }

    ~HttpRequest() = default;

    void init();

    bool parse(Buffer &buffer);

    std::string path() const;
    
    std::string &path();

    std::string method() const;

    std::string version() const;

    std::string get_post(const std::string &key) const;
    std::string get_post(const char *key) const;

    bool is_keep_alive() const;

private:
    /**
     * parse a request line of an HTTP request
     * @param line a line to be parsed
    */
    bool parse_request_line(const std::string &line);

    /**
     * parse a request header of a HTTP request
     * @param line a line to be parsed
    */
    void parse_header(const std::string &line);

    /**
     * parse a request body of a HTTP request
     * @param line a line to be parsed
    */
    void parse_body(const std::string &line);

    /**
     * parse a request path of a HTTP request
    */
    void parse_path();
    /**
     * parse a POST HTTP request data when the Content-type is a specific string
    */
    void parse_post();

    /**
     * parse the body of a POST request that is encoded as a specific type
    */
    void parse_from_urlencoded();
    /**
     * verify a user's credentials against a MYSQL database. The method can handle both login
     * and registration scenarios
     * @param name name of the user
     * @param pwd password of the user
     * @param is_login whether the user is login or registration
     * @return 
    */
    static bool user_verify(const std::string &name, const std::string &pwd, bool is_login);

    /**
     * convert hex-number to dec-number
     * @param ch character to be converted
     * @return decimal number
    */
    static int conver_hex(char ch);

private:
    /**
     * the current state of the request parsing 
    */
    PARSE_STATE_ state_;

    /**
     * store the request path, HTTP method (e.g. GET, POST),  request body
    */
    std::string method_, path_, version_, body_;

    /**
     * a map that stores the HTTP headers and their values
    */
    std::unordered_map<std::string, std::string> header_;

    /**
     * a map that stores the parsed POST data from the request body
    */
    std::unordered_map<std::string, std::string> post_;

    /**
     * store a map that associates specific HTML file paths with integer tags which are common
     * endpoints that the server expects to handle, such as "/idnex", "/register", "/login",
     * "/welcome", "/video" and "/picture". this set is used to check and append the ".html"
     * extension if the requested path matches one of these default paths
    */
    static const std::unordered_set<std::string> DEFAULT_HTML_;

    /**
     * store a map that associates specific HTML file paths with integer tags
    */
    static const std::unordered_map<std::string, int> DEFAULT_HEML_TAG_;
};

#endif