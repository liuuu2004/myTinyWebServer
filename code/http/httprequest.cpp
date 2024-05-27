#include "httprequest.h"
#include <cassert>
#include <cstdio>
#include <mysql/mysql.h>
#include <regex>
#include <strings.h>

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

bool HttpRequest::user_verify(const std::string &name, const std::string &pwd, bool is_login) {
    if (name == "" || pwd == "") {
        // if name or password is empty, return false immediately
        return false;
    }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());

    // init a MYSQL connection and assert the connection is valid
    MYSQL *sql;
    SqlConnRAII(&sql, SqlConnPool::instance());
    assert(sql != nullptr);
    
    bool flag = false;
    // a number to store the number of fields in the result set
    unsigned int j = 0;
    // a character array to store the mysql query
    char order[256] = {0};
    // a pointer to store field metadata
    MYSQL_FIELD *fields = nullptr;
    // a pointer to store the result set
    MYSQL_RES *res = nullptr;

    if (!is_login) {
        flag = true;
    }
    /**
     * generate a SQL query to select the username and password from the user table where the user
     * name matches the provided username
    */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1",
    name.c_str());
    LOG_DEBUG("%s", order);

    if (mysql_query(sql, order)) {
        // if failed to execute the query, free the result set and return false
        mysql_free_result(res);
        return false;
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password = row[1];
        if (is_login) {
            if (pwd == password) {
                flag = true;
            } else {
                flag = false;
                LOG_DEBUG("password error!");
            }
        } else {
            flag = false;
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    if (!is_login && flag == true) {
        LOG_DEBUG("register!");
        bzero(order, 256);
        snprintf(order, 256, "INSERT INTO user(username, password) VALUES('%s', '%s')",
        name.c_str(), pwd.c_str());
        LOG_DEBUG("%s", order);
        if (mysql_query(sql, order)) {
            LOG_DEBUG("Insert error!");
            flag = false;
        }
        flag = true;
    }
    SqlConnPool::instance()->free_conn(sql);
    LOG_DEBUG("User Verify Success!");
    return flag;
}

int HttpRequest::conver_hex(char ch) {
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return ch;
}