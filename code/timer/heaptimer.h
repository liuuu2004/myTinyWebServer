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

    void adjust(int id, int new_expires);

    void add(int id, int time_out, const TimeoutCallBack &tcb);

    void do_work(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

private:
    /**
     * remove the TimerNode at the specified index from the heap by swapping it with the
     * last element and adjust the heap
     * @param i index of the TimerNode to be deleted
    */
    void del_(size_t i);

    /**
     * move the TimerNode at index i up to the heap to maintain the min-heap
     * @param i index of the TimerNode to be moved upper
    */
    void shift_up_(size_t i);

    /** 
     * move the TimerNode at index i down to maintain the min-heap
     * @param i the index of the TimerNode to be moved down
     * @param n TODO
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