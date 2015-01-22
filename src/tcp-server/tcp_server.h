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

#include <exception>

#include <boost/signals2.hpp>
#include <boost/bind.hpp>

#include "epoll_handler.h"
#include "tcp_socket.h"

struct tcp_server
{
public:
    tcp_server(const std::string& address, const std::string& service, epoll_handler& handler, int max_pending_connections);
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
    boost::signals2::signal<void (tcp_socket&)> on_connection;
    boost::signals2::signal<void (const std::exception&)> on_error;
    void accept_connection(tcp_socket& socket);
    void proceed_connection(tcp_socket& socket);
};

#endif //TCP_SERVER_H
