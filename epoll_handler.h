#ifndef EPOLL_HANDLER_H
#define EPOLL_HANDLER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <stdint.h>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <exception>
#include <map>

#include "tcp_socket.h"

struct tcp_socket;

struct epoll_handler
{
public:
    epoll_handler();
    ~epoll_handler();
    void start();
    void stop();
    void add(tcp_socket* socket);
private:
    static const int MAX_EVENTS = 64;
    static constexpr const char* END_STR = "1";

    std::map<int, tcp_socket*> sockets;
    epoll_event* events;
    int event_fd_;
    int epoll_fd_;
    void create_event_fd();
};

#endif // EPOLL_HANDLER_H
