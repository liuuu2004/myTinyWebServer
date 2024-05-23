#ifndef THREADPOOL
#define THREADPOOL

#include <cassert>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
public:
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    /**
     * create a thread pool with thread number as a specified number to execute tasks
     * concurrently
     * @param thread_num the number of threads to execute tasks in the thread pool
    */
    explicit ThreadPool(size_t thread_num = 8) : pool_(std::make_shared<Pool>()) {
        assert(thread_num > 0);
        for (auto i = 0; i < thread_num; i++) {
           std::thread([pool = pool_] {
                std::unique_lock<std::mutex> locker(pool->mutex_);
                while (true) {
                    /**
                     * why unlock -> task() -> lock() ?
                     * to ensure that the task execution doesnot holds the lock, this could:
                     *   1. avoid blocking other threads:
                     *       if the mutex remained locked while the task is executing, other
                     *       threads in the pool would be blocked from accessing other tasks.
                     *   2. prevent deadlocks:
                     *       if the task begin executed involves operations that might need
                     *       to acquire the same mutex, this may cause deadlocks.
                    */
                    if (!pool->tasks_.empty()) {
                        auto task = std::move(pool->tasks_.front());
                        pool->tasks_.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    } else if (pool->is_closed_) {
                        break;
                    } else {
                        pool->cond_.wait(locker);
                    }
                }
           }).detach();
        }
    }
private:
    struct Pool {
        std::mutex mutex_;
        std::condition_variable cond_;
        bool is_closed_;
        std::queue<std::function<void()>> tasks_;
    };
    std::shared_ptr<Pool> pool_;
};


#endif