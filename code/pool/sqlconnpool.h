#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mutex>
#include <queue>
#include <mysql/mysql.h>
#include <semaphore.h>

class SqlConnPool {
public:
    static SqlConnPool *instance();
    MYSQL *get_conn();
    void free_conn();
    int get_free_conn_count();

    void init(const char *host, int port, const char *user,
              const char *pwd, const char *db_name, int conn_size);
    void close_pool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;
    int use_count_;
    int free_count_;

    std::queue<MYSQL *> conn_queue_;
    std::mutex mutex_;
    sem_t sem_id_;
};


#endif