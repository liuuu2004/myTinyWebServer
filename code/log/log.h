#ifndef LOG_H
#define LOG_H

#include "../buffer/buffer.h"
#include "blockqueue.h"
#include <memory>
#include <thread>

class Log {
public:
    void init(int level, const char *path = "./log", const char *suffix = "./log",
              int max_capacity = 1024);
    static Log *instance();
    static void FlushLogThread();

    void write(int level, const char *format, ...);
    void flush();

    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();

private:
    /**
     * initialize member variables
    */
    Log();
    void AppendLogLevelTitle(int level);

    /**
     * ensure any running threads is joined and that the log file is flushed and closed
    */
    virtual ~Log();
    void AsyncWrite();

    static const int LOG_PATH_INT = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LENS = 50000;

    const char *path_;
    const char *suffix_;

    int MAX_LINES;
    
    int line_count_;
    int today_;
    bool is_open_;
    Buffer buffer_;
    int level_;
    bool is_async_;

    FILE *fp_;
    std::unique_ptr<std::thread> write_thread_;
    std::unique_ptr<BlockDeque<std::string>> deque_;
    std::mutex mutex_;
};


#endif