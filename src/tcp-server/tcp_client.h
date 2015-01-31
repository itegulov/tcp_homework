#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#include "epoll_handler.h"
#include "tcp_exception.h"
#include "tcp_socket.h"

#include "boost/signals2.hpp"

struct tcp_client {
public:
    tcp_client(const std::string address, const std::string service, epoll_handler& handler);
    void connect();

    template<typename T>
    void connect_on_connect(T function)
    {
        on_connect.connect(function);
    }

    template<typename T>
    void connect_on_message(T function)
    {
        on_message.connect(function);
    }

    template<typename T>
    void connect_on_error(T function)
    {
        on_error.connect(function);
    }

    void write(const std::string& data);
private:
    boost::signals2::signal<void (tcp_socket&)> on_connect;
    boost::signals2::signal<void (tcp_socket&)> on_message;
    boost::signals2::signal<void (const std::exception&)> on_error;

    static void *get_in_addr(struct sockaddr *sa);
    std::shared_ptr<tcp_socket> socket_;
    const std::string address_;
    const std::string service_;
    epoll_handler& handler_;
};
#endif //TCP_CLIENT_H
