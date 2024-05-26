#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

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

    ~HttpRequest();

    void init();

private:
    bool parse_request_line(const std::string &line);
    void parse_header(const std::string &line);
    void parse_body(const std::string &line);

    void parse_path();
    void parse_post();
    void parse_from_urlencoded();

    static bool user_verify(const std::string &name, const std::string &pwd, bool is_login);

private:
    PARSE_STATE_ state;
    std::string method_, path_, version_, body_;
    std::unordered_map<std::string, std::string> header_;
    std::unordered_map<std::string, std::string> post_;

    static const std::unordered_set<std::string> DEFAULT_HTML_;
    static const std::unordered_map<std::string, int> DEFAULT_HEML_TAG_;
    static int conver_hex(char ch);
};

#endif