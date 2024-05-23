#include "heaptimer.h"
#include <cassert>
#include <chrono>
#include <utility>

void HeapTimer::shift_up_(size_t i) {
    assert(i >= 0 && i < heap_.size());
    size_t j = (i - 1) / 2;
    while (j >= 0) {
        if (heap_[i] < heap_[j]) {
            break;
        }
        swap(i, j);
        i = j;
        j = (i - 1) / 2;
    }
}

bool HeapTimer::shift_down_(size_t i, size_t n) {
    assert(i >= 0 && i < heap_.size());
    assert(n >= 0 && n <= heap_.size());
    size_t j = i;
    size_t k = j * 2 + 1;
    while (k < n) {
        if (k + 1 < n && heap_[k + 1] < heap_[k]) {
            k++;
        }
        if (heap_[j] < heap_[k]) {
            break;
        }
        swap(j, k);
        j = k;
        k = j * 2 + 1;
    }
    return j > i;
}

void HeapTimer::swap(size_t i, size_t j) {
    assert(i >= 0 && i < heap_.size());
    assert(j >= 0 && j < heap_.size());
    std::swap(heap_[i], heap_[j]);
    ref_[heap_[i].id] = j;
    ref_[heap_[j].id] = i;
}

void HeapTimer::del(size_t i) {
    assert(!heap_.empty() && i >= 0 && i < heap_.size());
    size_t j = i;
    size_t n = heap_.size() - 1;
    assert(i <= n);
    if (j < n) {
        swap(j, n);
        if (!shift_down_(j, n)) {
            shift_up_(j);
        }
    }
    ref_.erase(heap_.back().id);
    heap_.pop_back();
}

void HeapTimer::adjust(int id, int time_out) {
    assert(!heap_.empty() && ref_.count(id) > 0);
    heap_[ref_[id]].expires = Clock::now() + MS(time_out);
    shift_down_(ref_[id], heap_.size());
}

void HeapTimer::add(int id, int time_out, const TimeoutCallBack &tcb) {
    assert(id >= 0);
    size_t i;

    if (ref_.count(id) == 0) {
        // if the node is new
        i = heap_.size();
        ref_[id] = i;
        heap_.push_back({id, Clock::now() + MS(time_out), tcb});
        shift_up_(i);
    } else {
        // if the node already exists
        i = ref_[id];
        heap_[i].expires = Clock::now() + MS(time_out);
        heap_[i].tcb = tcb;
        if (!shift_down_(i, heap_.size())) {
            shift_up_(i);
        }
    }
}

void HeapTimer::do_work(int id) {
    if (heap_.empty() || ref_.count(id) == 0) {
        return;
    }
    size_t i = ref_[id];
    TimerNode node = heap_[i];
    node.tcb();
    del(i);
}

void HeapTimer::clear() {
    ref_.clear();
    heap_.clear();
}

void HeapTimer::tick() {
    if (heap_.empty()) {
        return;
    }
    while (!heap_.empty()) {
        auto node = heap_.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;                                                                                   
        }
        node.tcb();
        pop();
    }
}

void HeapTimer::pop() {
    assert(!heap_.empty());
    del(0);
}

int HeapTimer::GetNextTick() {
    tick();
    size_t du = -1;
    if (!heap_.empty()) {
        du = std::chrono::duration_cast<MS>(heap_.front().expires - Clock::now()).count();
    }
    return du;
}