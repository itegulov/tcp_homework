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

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include <exception>

#include "tcp_socket.h"
#include "tcp_exception.h"

struct tcp_socket;
struct epoll_handler;

struct tcp_server
{
    friend struct tcp_socket;
public:
    ~tcp_server();
    void start(const char * address, const char * service, epoll_handler * handler);
    void set_max_pending_connections(int max);
    boost::signals2::signal<void (tcp_socket*)> new_connection;
    boost::signals2::signal<void (const std::exception&)> on_error;
private:
    int max_pending_connections_ = 16;

    tcp_socket* main_socket;

    void accept_connection(tcp_socket* socket, epoll_handler* handler);
    void proceed_connection(tcp_socket* socket, epoll_handler* handler);
};

#endif //TCP_SERVER_H
