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
    void del_(size_t i);
    void shift_up_(size_t i);
    bool shift_down_(size_t i, size_t n);
    void swap(size_t i, size_t j);

    std::vector<TimerNode> heap_;
    std::unordered_map<int, size_t> ref_;
};


#endif