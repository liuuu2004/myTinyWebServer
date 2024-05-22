#include "heaptimer.h"
#include <cassert>

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