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

#include <boost/signals2.hpp>

#include <cstring>

struct tcp_socket {
public:
    tcp_socket();
    tcp_socket(int fd);
    ~tcp_socket();
    boost::signals2::signal<void (tcp_socket*)> on_read;
    int get_socket_descriptor();
    void close_socket();
    bool is_open();
    int read_data(char* data, int max_size);
    int write_data(const char* data, int size);
    void bind_socket(const char * address, const char * service);
    void make_socket_non_blocking();
    void listen(int max_pending_connections);
    std::string read_all();
private:
    static const int CHUNK_SIZE = 512;
    int fd_;
    bool is_open_;
};

#endif //TCP_SOCKET_H
