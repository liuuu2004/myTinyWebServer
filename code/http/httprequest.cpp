#include "httprequest.h"
#include <regex>

bool HttpRequest::parse_request_line(const std::string &line) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch sub_smatch;

    if (std::regex_match(line, sub_smatch, patten)) {
        method_ = sub_smatch[1];
        path_ = sub_smatch[2];
        version_ = sub_smatch[3];
        state_ = HEADERS;
        return true;
    }
    LOG_ERROR("RequestLine Error!");
    return false;
}