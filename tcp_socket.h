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

    int get_descriptor() const;
    void close();
    bool is_open() const;
    void bind(const char * address, const char * service);
    void make_non_blocking();
    void listen(int max_pending_connections);

    int read_data(char* data, int max_size);
    int write_data(const char* data, int max_size) const;
    char* read_all();
    void write_all(const char* data, int size) const;

    boost::signals2::signal<void (tcp_socket*)> on_read;
private:
    static const int CHUNK_SIZE = 512;
    static const int RESULT_SIZE = 8192;
    static constexpr const char* EMPTY_STR = "";
    int fd_;
    bool is_open_;
};

#endif //TCP_SOCKET_H
