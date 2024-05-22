#include "log.h"
#include <cstdio>
#include <mutex>

Log::Log() {
    line_count_ = 0;
    is_async_ = false;
    write_thread_ = nullptr;
    deque_ = nullptr;
    today_ = 0;
    fp_ = nullptr;
}

Log::~Log() {
    if (write_thread_ != nullptr && write_thread_->joinable()) {
        while (!deque_->empty()) {
            deque_->flush();
        };
        deque_->close();
        write_thread_->join();
    }
    if (fp_ != nullptr) {
        std::lock_guard<std::mutex> locker(mutex_);
        flush();
        fclose(fp_);
    }
}