#include "httprequest.h"
#include <cassert>
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

void HttpRequest::parse_header(const std::string &line) {
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch sub_match;

    if (std::regex_match(line, sub_match, patten)) {
        header_[sub_match[1]] = sub_match[2];
    } else {
        state_ = BODY;
    }
}

void HttpRequest::parse_body(const std::string &line) {
    body_ = line;
    parse_post();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

void HttpRequest::parse_path() {
    if (path_ == "/") {
        path_ = "/index.html";
    } else {
        for (auto &item : DEFAULT_HTML_) {
            if (item == path_) {
                path_ += ".html";
                break;
            }
        }
    }
}

void HttpRequest::parse_post() {
    if (method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        parse_from_urlencoded();
        if (DEFAULT_HEML_TAG_.count(path_) != 0) {
            int tag = DEFAULT_HEML_TAG_.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if (tag == 0 || tag == 1) {
                bool is_login = (tag == 1);
                if (user_verify(post_["username"], post_["password"], is_login)) {
                    path_ = "/welcome.html";
                } else {
                    path_ = "/error.html";
                }
            }
        }
    }
}

void HttpRequest::parse_from_urlencoded() {
    if (body_.size() == 0) {
        return;
    }
    std::string key, value;
    int num = 0, n = body_.size(), i = 0, j = 0;
    for (; i < n; i++) {
        char ch = body_[i];
        switch (ch) {
            case '=':
                key = body_.substr(j, i - j);
                j = i + 1;
                break;
            case '+':
                body_[i] = ' ';
                break;
            case '%':
                num = conver_hex(body_[i + 1]) * 16 + conver_hex(body_[i + 2]);
                body_[i + 2] = num % 10 + '0';
                body_[i + 1] = num / 10 + '0';
                i += 2;
                break;
            case '&':
                value = body_.substr(j, i - j);
                j = i + 1;
                post_[key] = value;
                LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
                break;
            default:
                break;
        }
    }
    assert(j <= i);
    if (post_.count(key) == 0 && j < i) {
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}