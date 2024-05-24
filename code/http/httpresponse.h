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
    /**
     * init variables with specific value
     * @param src_dir diractor to store resources
     * @param path path to store resource
     * @param is_keep_alive shall we keep the connction alive for further requests
     * @param code HTTP reaponse code
    */
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