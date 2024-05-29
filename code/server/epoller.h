#ifndef EPOLLER_H_
#define EPOLLER_H_

#include <cstdint>
#include <vector>
#include <sys/epoll.h>


class Epoller {
public:
    /**
     * initialize the epoller_fd_ file descriptor by calling 'epoll_create' and initialize the
     * events_ vector to hold max_event number of epoll_event structures.
    */
    explicit Epoller(int max_event = 1024);

    /**
     * close file dexcriptor
    */
    ~Epoller();

    /**
     * adds a file descriptor to the epoll instance, monitoring it for the events secified in 'events'
     * @param fd file descriptor to be added
     * @param events events to be added
     * @return true if the operation is successful, otherwise false
    */
    bool add_fd(int fd, uint32_t events);

    /**
     * modifies the event mask for a file descriptor fd in the epoll instance
     * @param fd file descriptor to be modifued
     * @param events events to be modified
     * @return true if the operation is successful, otherwise false
    */
    bool mod_fd(int fd, uint32_t events);

    /**
     * deletes a file descriptor from the epoll instance
     * @param fd file descriptor to be deleted
     * @return true if the operation is successful, otherwise false 
    */
    bool del_fd(int fd);

    /**
     * waits for events on the file descriptors added to the epoll instance
     * @param timeout_ms specifies the maximum time to wait in ms
     * @return number of file descriptors ready for the requested IO, or -1 if an error occurs
    */
    int wait(int timeout_ms = -1);

    /**
     * return the file descriptor at the specific position
     * @param i position of the file descriptor
     * @return the file descriptor associated with the i th event in the 'events_' vector
    */
    int get_event_fd(size_t i) const;

    /**
     * get the events at the specific position
     * @param i position of the events
     * @return the event associated with the i th event in the 'events_' vector
    */
    uint32_t get_events(size_t i) const;
private:
    /**
     * hold the file descriptor for the epoll instance
    */
    int epoller_fd_;

    /**
     * stores theevents that are returned by the epoll_wait system call
    */
    std::vector<struct epoll_event> events_;
};

#endif