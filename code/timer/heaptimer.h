#ifndef HEAPTIMER_H
#define HEAPTIMER_H

#include <chrono>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <vector>

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode {
    int id;
    TimeStamp expires;
    TimeoutCallBack tcb;
    bool operator < (const TimerNode& node) {
        return expires < node.expires;
    }
};

class HeapTimer {
public:
    HeapTimer() {
        heap_.reserve(64);
    }

    ~HeapTimer() {
        clear();
    }

    /**
     * adjust the expiration time of a TimerNode with a specified id
     * @param id id of the TimerNode that is going to be adjusted
     * @param new_expires the new timeout duration in ms, to be added to the current
     *                    time to set the new expiration time
    */
    void adjust(int id, int new_expires);

    /**
     * add a new TimerNode or update an exsiting TimerNode in the heap
     * @param id the id of the TimerNode
     * @param time_out the timeout duration in ms
     * @param tcb the call-back function to be called when the timer expires
    */
    void add(int id, int time_out, const TimeoutCallBack &tcb);

    /**
     * delete TimerNode with specified id and call call-back function
     * @param id id of the TimerNode which is going to be deleted
    */
    void do_work(int id);

    /**
     * clear ref_ and heap_
    */
    void clear();

    /**
     * clear time-out TimerNodes
    */
    void tick();

    /**
     * pop the top TimerNode
    */
    void pop();

    /**
     * get the duration time in ms until the next timer events shall expire
     * @return if no next timer, return -1, else return the duration time in
     *         ms until the next timer events shall expire
    */
    int GetNextTick();

private:
    /**
     * remove the TimerNode at the specified index from the heap by swapping it with the
     * last element and adjust the heap
     * @param i index of the TimerNode to be deleted
    */
    void del(size_t i);

    /**
     * move the TimerNode at index i up to the heap to maintain the min-heap
     * @param i index of the TimerNode to be moved upper
    */
    void shift_up_(size_t i);

    /** 
     * move the TimerNode at index i down to maintain the min-heap
     * @param i the index of the TimerNode to be moved down
     * @param n the number of TimerNodes should be considered in the shift down
    */
    bool shift_down_(size_t i, size_t n);

    /**
     * swap the position of the two TimerNodes by their index
     * @param i node1 index
     * @param j node2 index
    */
    void swap(size_t i, size_t j);

    /**
     * an containner to store all the TimerNodes
    */
    std::vector<TimerNode> heap_;

    /**
     * map an id to its position in the map
    */
    std::unordered_map<int, size_t> ref_;
};


#endif