#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>


template<class T>
class BlockDeque {
public:
    /**
     * define the max capacity of the block deque as something, default 1000
     * and set is_closed_ to false
     * @param max_capacity the maximum capacity of the block deque
    */
    explicit BlockDeque(size_t max_capacity = 1000);

    /**
     * call clear() to safely destruc the block deque
    */
    ~BlockDeque();

    /**
     * clear all elements in the block deque
    */
    void clear();

    /**
     * check whether the block deque is empty
     * @return whether the block deque is empty
    */
    bool empty();

    /**
     * checker whether the block deque is full
     * @return whether the block deque is full
    */
    bool full();

    /**
     * safely shut down the block deque by:
     *    1. locking the mutex to ensure thread-safe access
     *    2. clearing the deque of all its elenents
     *    3. setting a flag to indicate that the deque is closed
     *    4. notifying all producer and consumer threads to terminate their
     *       possibly waiting
    */
    void close();

    /**
     * get the size of the block deque
     * @return the size of the block deque
    */
    size_t size();

    /**
     * get the capacity of the block deque
     * @return the capaciy of the block deque
    */
    size_t capacity();

    /**
     * get the front element of the block deque
     * @return the front element of the block deque
    */
    T front();

    /**
     * get the back element of the block deque
     * @return the back element of the block deque
    */
    T back();

    /**
     * push item to the back of the block deque
     * @param item the item which will be pushed to the back of the block deque
    */
    void push_back(T &item);

    /**
     * push item to the front of the block deque
     * @param item the item which will be pushed to the front of the block deque
    */
    void push_front(T &item);

    /**
     * pop the front item and store its value to item
     * @param item where the popped item to be stored
     * @return whether the pop successed
    */
    bool pop(T &item);

    /**
     * pop the front item and store its value to item, if didnot response in a specified
     * time, return false.
     @param item where the popped item to be stored
     @param timeout after this time without popping, return false
     @param whether the pop successed on time
    */
    bool pop(T &item, int timeout);

    /**
     * signal a waiting consumer that there might be new data availiable or it should check the
     * state of the deque
    */
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