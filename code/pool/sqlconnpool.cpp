#include "sqlconnpool.h"
#include <cassert>
#include <mysql/mysql.h>
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