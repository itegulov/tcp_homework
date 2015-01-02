#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "tcp_server_api.h"

#include <boost/signals2.hpp>

#include <cstring>

struct tcp_socket {
    friend struct tcp_client;
    friend struct tcp_server;
    friend struct epoll_handler;
public:
    tcp_socket(int fd, epoll_handler& handler);
    tcp_socket(const tcp_socket& other);
    tcp_socket();
    ~tcp_socket();

    int get_descriptor() const;
    void close();
    bool is_open() const;
    void bind(const std::string& address, const std::string& service);
    void make_non_blocking();
    void listen(int max_pending_connections);

    std::string read_data(ssize_t max_size);
    ssize_t write_data(const std::string & data) const;
    std::string read_all();
    void write_all(const std::string & data) const;

    template<typename T>
    void connect_on_read(T function) {
        on_read.connect(function);
    }

    template<typename T>
    void connect_on_epoll(T function)
    {
        on_epoll.connect(function);
    }

    template<typename T>
    void connect_on_close(T function)
    {
        on_close.connect(function);
    }

private:
    static const ssize_t CHUNK_SIZE = 512;
    static const ssize_t RESULT_SIZE = 8192;
    static constexpr const char* EMPTY_STR = "";

    epoll_handler& handler_;

    boost::signals2::signal<void (tcp_socket&)> on_close;
    boost::signals2::signal<void (tcp_socket&)> on_read;
    boost::signals2::signal<void (tcp_socket&)> on_epoll;

    int fd_;
    bool is_open_;
};

#endif //TCP_SOCKET_H
