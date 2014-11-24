#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#include <boost/signals2.hpp>

struct tcp_socket {
public:
    tcp_socket(int fd);
    ~tcp_socket();
    boost::signals2::signal<void (tcp_socket*)> on_read;
    int get_socket_descriptor();
    void close_socket();
private:
    int fd;
};
#endif //TCP_SOCKET_H
