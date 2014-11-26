#ifndef TCP_SERVER_H
#define TCP_SERVER_H
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

#include <boost/signals2.hpp>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <exception>
#include <map>

#include "tcp_socket.h"
#include "tcp_exception.h"

struct tcp_server
{
public:
    ~tcp_server();
    bool begin_listening(const char * address, const char * service);
    void stop_listening();
    void set_max_pending_connections(int max);
    boost::signals2::signal<void (tcp_socket*)> new_connection;
private:
    static const int MAX_EVENTS = 64;
    int max_pending_connections_ = 16;
    int epoll_fd_;
    int event_fd_;
    bool is_running_ = false;
    std::thread* thread_;
    void create_event_fd();
    void run(tcp_socket* socket_fd, epoll_event* events);
};
#endif //TCP_SERVER_H

