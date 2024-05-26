#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>

#include "../buffer/buffer.h"
#include "../log/log.h"

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
    */
    bool parse_request_line(const std::string &line);
    void parse_header(const std::string &line);
    void parse_body(const std::string &line);

    void parse_path();
    void parse_post();
    void parse_from_urlencoded();

    static bool user_verify(const std::string &name, const std::string &pwd, bool is_login);

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