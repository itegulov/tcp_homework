#include "tcp_socket.h"

tcp_socket::tcp_socket(int fd)
{
    this->fd = fd;
}

tcp_socket::~tcp_socket()
{
    close_socket();
}

void tcp_socket::close_socket()
{
    close(fd);
}

int tcp_socket::get_socket_descriptor()
{
    return fd;
}
