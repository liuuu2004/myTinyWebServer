#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>


template<class T>
class BlockDeque {
public:
    explicit BlockDeque(size_t max_capacity = 1000);

    ~BlockDeque();

    void clear();

    bool empty();

    bool full();

    void close();

    size_t size();

    size_t capacity();

    T front();

    T back();

    void push_back(T &item);

    void push_front(T &item);

    void pop(T &item);

    void pop(T &item, int timeout);

    void flush();

private:
    /**
     * an underlying container (double-ended queue) that stores the elements of the block deque.
    */
    std::deque<T> deq_;

    /**
     * the maximum number of elements that the block deque can contain
    */
    size_t capacity_;

    /**
     * used to synchronize access to the block deque. it ensures that only one thread can modify
     * the block deque at a time, providing thread-safety
    */
    std::mutex mutex_;

    /**
     * a flag that indicates whether the block deque is closed
    */
    bool is_closed_;

    /**
     * used to notify consumer threads when elements are available in the block deque
    */
    std::condition_variable cond_consumer_;

    /**
     * used to notify producer threads when there is space available in th block deque
    */
    std::condition_variable cond_producer_;
};


#endif