#include "log.h"
#include "blockqueue.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

const char *Log::LogLevelStr[LEVEL_COUNT] = {
    "[DEBUG]: ",
    "[INFO]: ",
    "[WARN]: ",
    "[ERROR]: "
};

Log::Log() {
    line_count_ = 0;
    is_async_ = false;
    write_thread_ = nullptr;
    deque_ = nullptr;
    today_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    // empty the deque, join the write thread to close
    if (write_thread_ != nullptr && write_thread_->joinable()) {
        while (!deque_->empty()) {
            deque_->flush();
        };
        deque_->close();
        write_thread_->join();
    }
    // close file
    if (fp_ != nullptr) {
        std::lock_guard<std::mutex> locker(mutex_);
        flush();
        fclose(fp_);
    }
}

void Log::AppendLogLevelTitle(int level) {
    switch (level) {
        case DEBUG:
            buffer_.Append(LogLevelStr[level], 9);
        case INFO:
            buffer_.Append(LogLevelStr[INFO], 9);
        case WARN:
            buffer_.Append(LogLevelStr[WARN], 9);
        case ERROR:
            buffer_.Append(LogLevelStr[ERROR], 9);
        default:
            buffer_.Append(LogLevelStr[INFO], 9);
            break;
    }
}

void Log::AsyncWrite() {
    std::string str = "";
    while (deque_->pop(str)) {
        std::lock_guard<std::mutex> locker(mutex_);
        fputs(str.c_str(), fp_);
    }
}

void Log::init(int level, const char *path, const char *suffix, int max_capacity) {
    is_open_ = true;
    level_ = level;
    if (max_capacity > 0) {
        is_async_ = true;
        if (deque_ == nullptr) {
            std::unique_ptr<BlockDeque<std::string>> new_deque(new BlockDeque<std::string>);
            deque_ = std::move(new_deque);
            std::unique_ptr<std::thread> new_thread(new std::thread(FlushLogThread));
            write_thread_ = std::move(new_thread);
        }
    } else {
        is_async_ = false;
    }
    line_count_ = 0;
    time_t timer = time(nullptr);
    struct tm *sys_time = localtime(&timer);
    struct tm t = *sys_time;

    path_ = path;
    suffix_ = suffix;

    char file_name[LOG_NAME_LEN] = {0};
    snprintf(file_name, LOG_NAME_LEN, "%s/%04d_%02d_%02d_%s",
             path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    today_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mutex_);
        buffer_.RetrieveAll();
        if (fp_ != nullptr) {
            flush();
            fclose(fp_);
        }
        fp_ = fopen(file_name, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(file_name, "a");
        }
        assert(fp_ != nullptr);
    }
}

Log *Log::instance() {
    static Log instance;
    return &instance;
}

void Log::FlushLogThread() {
    Log::instance()->AsyncWrite();
}

void Log::flush() {
    if (is_async_) {
        deque_->flush();
    }
    fflush(fp_);
}

int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mutex_);
    return level_;
}

void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mutex_);
    level_ = level;
}

bool Log::IsOpen() {
    std::lock_guard<std::mutex> locker(mutex_);
    return is_open_;
}

void Log::write(int level, const char *format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t_sec = now.tv_sec;
    struct tm *sts_time = localtime(&t_sec);
    struct tm t = *sts_time;
    va_list vl;

    if (today_ != t.tm_mday || (line_count_ && (line_count_ % MAX_LINES == 0))) {
        std::unique_lock<std::mutex> locker(mutex_);
        locker.unlock();

        char new_file[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d",
                 t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        
        if (today_ != t.tm_mday) {
            snprintf(new_file, LOG_NAME_LEN - 72, "%s/%s%s",
                     path_, tail, suffix_);
            today_ = t.tm_mday;
            line_count_ = 0;
        } else {
            snprintf(new_file, LOG_NAME_LEN, "%s/%s-%d%s",
                     path_, tail, (line_count_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(new_file, "a");
        assert(fp_ != nullptr);
    }

    {
        std::unique_lock<std::mutex> locker(mutex_);
        line_count_++;
        int n = snprintf(buffer_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buffer_.HasWritten(n);
        AppendLogLevelTitle(level);

        va_start(vl, format);
        int m = vsnprintf(buffer_.BeginWrite(), buffer_.WritableBytes(), format, vl);
        va_end(vl);

        buffer_.HasWritten(m);
        buffer_.Append("\n\0", 2);

        if (is_async_ && deque_ != nullptr && !deque_->full()) {
            std::string s = buffer_.RetrieveAllToString();
            deque_->push_back(s);
        } else {
            fputs(buffer_.Peek(), fp_);
        }
        buffer_.RetrieveAll();
    }
}