#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "tcp_exception.h"
#include "tcp_server.h"
#include "epoll_handler.h"

#include <boost/signals2.hpp>

#include <cstring>

struct epoll_handler;

struct tcp_socket {
    friend struct tcp_server;
public:
    tcp_socket();
    tcp_socket(int fd);

    ~tcp_socket();

    int get_descriptor() const;
    void close();
    bool is_open() const;
    void bind(const char * address, const char * service);
    void make_non_blocking();
    void listen(int max_pending_connections);

    ssize_t read_data(char* data, ssize_t max_size);
    ssize_t write_data(const char* data, ssize_t max_size) const;
    const char* read_all();
    void write_all(const char* data, ssize_t size) const;

    boost::signals2::signal<void (tcp_socket*)> on_read;
    boost::signals2::signal<void (tcp_socket*, epoll_handler*)> on_epoll;
private:
    static const ssize_t CHUNK_SIZE = 512;
    static const ssize_t RESULT_SIZE = 8192;
    static constexpr const char* EMPTY_STR = "";

    int fd_;
};

#endif //TCP_SOCKET_H
