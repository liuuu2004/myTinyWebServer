#include "sqlconnpool.h"
#include <cassert>
#include <mutex>
#include <mysql/mysql.h>
#include <mysql/mysql/client_plugin.h>
#include <semaphore.h>

SqlConnPool::SqlConnPool() {
    use_count_ = 0;
    free_count_ = 0;
}

SqlConnPool::~SqlConnPool() {
    close_pool();
}

SqlConnPool *SqlConnPool::instance() {
    static SqlConnPool conn_pool;
    return &conn_pool;
}

void SqlConnPool::init(const char *host, int port, const char *user,
                       const char *pwd, const char *db_name, int conn_size) {
    assert(conn_size > 0);
    for (int i = 0; i < conn_size; i++) {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (sql == nullptr) {
            LOG_ERROR("MySQL Init Error!");
            assert(sql != nullptr);
        }
        sql = mysql_real_connect(sql, host, user, pwd, db_name, port, nullptr, 0);
        if (sql == nullptr) {
            LOG_ERROR("MySQL Connect Error!");
        }
        conn_queue_.push(sql);
    }
    MAX_CONN_ = conn_size;
    sem_init(&sem_id_, 0, MAX_CONN_);
}

MYSQL *SqlConnPool::get_conn() {
    MYSQL *sql = nullptr;
    if (conn_queue_.empty()) {
        LOG_WARN("SQL Connection Pool Busy!");
        return nullptr;
    }
    sem_wait(&sem_id_);
    {
        std::lock_guard<std::mutex> locker(mutex_);
        sql = conn_queue_.front();
        conn_queue_.pop();
    }
    return sql;
}

void SqlConnPool::free_conn(MYSQL *sql) {
    assert(sql != nullptr);
    std::lock_guard<std::mutex> locker(mutex_);
    conn_queue_.push(sql);
    sem_post(&sem_id_);
}

int SqlConnPool::get_free_conn_count() {
    std::lock_guard<std::mutex> locker(mutex_);
    return conn_queue_.size();
}

void SqlConnPool::close_pool() {
    std::lock_guard<std::mutex> locker(mutex_);
    while (!conn_queue_.empty()) {
        auto item = conn_queue_.front();
        conn_queue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}