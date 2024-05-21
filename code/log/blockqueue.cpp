#include "blockqueue.h"
#include <cassert>
#include <mutex>

template<class T>
BlockDeque<T>::BlockDeque(size_t max_capacity) : capacity_(max_capacity) {
    assert(max_capacity > 0);
    is_closed_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque() {
    close();
}

template<class T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(mutex_);
        deq_.clear();
        is_closed_ = true;
    }
    cond_consumer_.notify_all();
    cond_producer_.notify_all();
}

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mutex_);
    deq_.clear();
}

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.empty();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.size() >= capacity_;
}

template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mutex_);
    return capacity_;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.back();
}

template<class T>
void BlockDeque<T>::push_back(T &item) {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.push_back(item);
}

template<class T>
void BlockDeque<T>::push_front(T &item) {
    std::lock_guard<std::mutex> locker(mutex_);
    return deq_.push_front(item);
}