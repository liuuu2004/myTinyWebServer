/**
 *  needs a introduction to sem_t...
*/

#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mutex>
#include <queue>
#include <mysql/mysql.h>
#include <semaphore.h>
#include "../log/log.h"

class SqlConnPool {
public:
    /**
     * get a sql connect pool instance (create and return one)
     * @return a sql connect pool instance
    */
    static SqlConnPool *instance();

    /**
     * add a sql connection to connect queue
     * @return sql connection
    */
    MYSQL *get_conn();

    /**
     * free a specific sql connection in the connection queue
    */
    void free_conn(MYSQL *sql);

    /**
     * get the number of free connections
     * @return number of free connection
    */
    int get_free_conn_count();

    /**
     * init a sql connection with specific host, port, user, pwd, db name
     * and set the default connection size to 10
     * @param host host of the DB
     * @param port port of the DB
     * @param user user of the DB
     * @param pwd pwd of the DB
     * @param dn_name name of the DB
     * @param conn_size max num of the sql connection, default 10
    */
    void init(const char *host, int port, const char *user,
              const char *pwd, const char *db_name, int conn_size = 10);
    
    /**
     * pop all elements in the connction queue and close mysql
    */
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