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
struct tcp_server;

struct tcp_socket {
    friend struct tcp_server;
    friend struct epoll_handler;
public:
    tcp_socket(tcp_server* server);
    tcp_socket(int fd, tcp_server* server);

    ~tcp_socket();

    int get_descriptor() const;
    void close();
    bool is_open() const;
    void bind(const char * address, const char * service);
    void make_non_blocking();
    void listen(int max_pending_connections);

    ssize_t read_data(char* data, ssize_t max_size);
    ssize_t write_data(const char* data, ssize_t max_size) const;
    std::string read_all();
    void write_all(const char* data, ssize_t size) const;

    template<typename T>
    void connect_on_read(T function) {
        on_read.connect(function);
    }

    template<typename T>
    void connect_on_epoll(T function)
    {
        on_epoll.connect(function);
    }

private:
    static const ssize_t CHUNK_SIZE = 512;
    static const ssize_t RESULT_SIZE = 8192;
    static constexpr const char* EMPTY_STR = "";

    tcp_server* server;

    boost::signals2::signal<void (tcp_socket*)> on_read;
    boost::signals2::signal<void (tcp_socket*)> on_epoll;

    int fd_;
};

#endif //TCP_SOCKET_H
