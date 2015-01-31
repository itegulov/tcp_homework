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

#include "boost/signals2.hpp"

#include "tcp_server_api.h"

struct epoll_handler
{
public:
    epoll_handler();
    ~epoll_handler();

    void start();
    void stop();
    void add(std::shared_ptr<tcp_socket> socket);

    template<typename T>
    void connect_on_error(T function)
    {
        on_error.connect(function);
    }

private:
    static const int MAX_EVENTS = 64;
    static constexpr const char* END_STR = "1";

    boost::signals2::signal<void (tcp_socket&)> on_error;

    std::map<int, std::shared_ptr<tcp_socket> > sockets;
    int epoll_fd_;
    int event_fd_;
    bool running_ = false;

    void create_event_fd();
};

#endif // EPOLL_HANDLER_H
