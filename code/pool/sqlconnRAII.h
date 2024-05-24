#ifndef SQLCONNRAII_H_
#define SQLCONNRAII_H_

#include "sqlconnpool.h"
#include <cassert>

class SqlConnRAII {
public:
    SqlConnRAII(MYSQL **sql, SqlConnPool *conn_pool) {
        assert(conn_pool != nullptr);
        *sql = conn_pool->get_conn();
        sql_ = *sql;
        conn_pool_ = conn_pool;
    }

    ~SqlConnRAII() {
        if (sql_ != nullptr) {
            conn_pool_->free_conn(sql_);
        }
    }
    
private:
    MYSQL *sql_;
    SqlConnPool *conn_pool_;
};

#endif