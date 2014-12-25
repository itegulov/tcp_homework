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

#include "tcp_exception.h"
#include "tcp_socket.h"

#include "boost/signals2.hpp"

struct tcp_client {
public:
    void tcp_connect(const char * address, const char * service);
    boost::signals2::signal<void (tcp_socket*)> on_connect;

    void write(const char* data, ssize_t size);
private:
    static void *get_in_addr(struct sockaddr *sa);
    tcp_socket socket_;
};
#endif //TCP_CLIENT_H
