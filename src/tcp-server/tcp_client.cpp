#include "tcp_client.h"

#include <iostream>

void *tcp_client::get_in_addr(sockaddr *sa)
{
    return &(((sockaddr_in*)sa)->sin_addr);
}

void tcp_client::tcp_connect(const char * address, const char * service)
{
    addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address, service, &hints, &servinfo);
    if (status != 0)
    {
        throw tcp_exception(gai_strerror(status));
    }
    int socket_fd;

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1)
        {
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        throw tcp_exception("Failed to connect");
    }
    socket_.fd_ = socket_fd;

    freeaddrinfo(servinfo);
    std::cout << "on_connect: yay" << std::endl;
    on_connect(&socket_);
}

void tcp_client::write(const char *data, ssize_t size)
{
    socket_.write_all(data, size);
}
