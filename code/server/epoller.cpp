#include "epoller.h"
#include <cassert>
#include <cstdint>
#include <sys/epoll.h>
#include <unistd.h>

Epoller::Epoller(int max_event) : epoller_fd_(epoll_create(512)), events_(max_event) {
    assert(epoller_fd_ >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epoller_fd_);
}

bool Epoller::add_fd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::mod_fd(int fd, uint32_t events) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::del_fd(int fd) {
    if (fd < 0) {
        return false;
    }
    epoll_event ev = {0};
    return 0 == epoll_ctl(epoller_fd_, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeout_ms) {
    return epoll_wait(epoller_fd_, &events_[0], static_cast<int>(events_.size()), timeout_ms);
}

int Epoller::get_event_fd(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd;
}

uint32_t Epoller::get_events(size_t i) const {
    assert(i < events_.size() && i >= 0);
    return events_[i].events;
}