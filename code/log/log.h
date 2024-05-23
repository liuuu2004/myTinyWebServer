#ifndef LOG_H
#define LOG_H

#include "../buffer/buffer.h"
#include "blockqueue.h"
#include <memory>
#include <thread>
#include <sys/stat.h>
#include <sys/time.h>

class Log {
public:
    enum LogLevel {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR
    };
    static const int LEVEL_COUNT = 4;
    static const char *LogLevelStr[LEVEL_COUNT];
    
    /**
     * initialize the logging system, set up asynchronous logging if specified, manage
     * log file creation and ensure thread safety during file operations
     * @param level log level  i.e. LOG, DEBUG...
     * @param path path of the log file
     * @param suffix refers to the file extension or ending part of the log file name
     * @param max_capacity the max capacity of the block deque 
    */
    void init(int level = INFO, const char *path = "./log", const char *suffix = "./log",
              int max_capacity = 1024);

    /**
     * get a log instance
     * @return log instance
    */
    static Log *instance();

    /**
     * serve as the entry point for the asynchronous logging mechanism
    */
    static void FlushLogThread();

    /**
     * @param level log level
     * @param format format the log message
    */
    void write(int level, const char *format, ...);

    /**
     * ensure all the log data is written to the log file
    */
    void flush();

    /**
     * get the level of this log
     * @return level of the log
    */
    int GetLevel();

    /**
     * set the log level
     * @param level log level to be set
    */
    void SetLevel(int level);

    /**
     * check whether the log is open
     * @return whether the og is open
    */
    bool IsOpen() {return is_open_; };

private:
    /**
     * initialize member variables
    */
    Log();

    /**
     * append a log level title to buffer based on the log level
     * @param level log level  i.e. INFO, DEBUG, ERROR...
    */
    void AppendLogLevelTitle(int level);

    /**
     * ensure any running threads is joined and that the log file is flushed and closed
    */
    virtual ~Log();

    /**
     * asynchronously write log message from a block deque to file
    */
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

#define LOG_BASE(level, format, ...) \
    do { \
        Log *log = Log::instance();\
        if (log->IsOpen() && log->GetLevel() <= level) { \
            log->write(level, format, ##__VA_ARGS__); \
            log->flush(); \
        } \
    } while (0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(Log::LogLevel::DEBUG, format, ##__VA_ARGS__)} while (0);
#define LOG_INFO(format, ...) do {LOG_BASE(Log::LogLevel::INFO, format, ##__VA_ARGS__)} while (0);
#define LOG_WARN(format, ...) do {LOG_BASE(Log::LogLevel::WARN, format, ##__VA_ARGS__)} while (0);
#define LOG_ERROR(format, ...) do {LOG_BASE(Log::LogLevel::ERROR, format, ##__VA_ARGS__)} while (0);


#endif