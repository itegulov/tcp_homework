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
    tcp_server(const char * address, const char * service, epoll_handler * handler, int max_pending_connections_);
    ~tcp_server();

    template<typename T>
    void connect_on_connection(T function)
    {
        on_connection.connect(function);
    }

    template<typename T>
    void connect_on_error(T function)
    {
        on_error.connect(function);
    }

private:
    boost::signals2::signal<void (tcp_socket*)> on_connection;
    boost::signals2::signal<void (const std::exception&)> on_error;
    void accept_connection(tcp_socket* socket);
    void proceed_connection(tcp_socket* socket);

    epoll_handler* handler;
};

#endif //TCP_SERVER_H
