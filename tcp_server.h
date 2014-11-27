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
    friend struct tcp_socket;
public:
    ~tcp_server();
    void start(const char * address, const char * service);
    void stop();
    void set_max_pending_connections(int max);
    boost::signals2::signal<void (tcp_socket*)> new_connection;
    boost::signals2::signal<void (const std::exception&)> on_error;
private:
    static bool socket_deleted;
    static const int MAX_EVENTS = 64;
    static constexpr const char * END_STR = "1";

    int max_pending_connections_ = 16;
    int epoll_fd_;
    int event_fd_;
    bool is_running_ = false;

    std::thread* thread_;

    void create_event_fd();
    void run(tcp_socket* socket_fd, epoll_event* events);
};

#endif //TCP_SERVER_H

