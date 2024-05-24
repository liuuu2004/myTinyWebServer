#ifndef HTTP_RESPONSE_H_
#define HTTP_RESPONSE_H_

#include <string>
#include <fcntl.h>
#include <unordered_map>

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();
    void init(const std::string &src_dir, std::string &path, bool is_keep_alive = false, int code = -1);
    void make_response(Buffer &buffer);
    void unmap_file();
    char *file();
    size_t file_len() const;
    void error_content(Buffer &buffer, std::string message);
    int code() const;

private:  // methods
    void add_state_line(Buffer &buffer);
    void add_header(Buffer &buffer);
    void add_content(Buffer &buffer);
    void error_html();
    std::string get_file_type();
    
private:  // variables
    int code_;
    bool is_keep_alive_;
    std::string path_;
    std::string src_dir_;

    char *mm_file_;
    struct stat mm_file_stat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE_;
    static const std::unordered_map<int, std::string> CODE_STATUS_;
    static const std::unordered_map<int, std::string> CODE_PATH_;
};

#endif