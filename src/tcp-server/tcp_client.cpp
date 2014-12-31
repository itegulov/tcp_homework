#include "tcp_client.h"

#include <iostream>

void *tcp_client::get_in_addr(sockaddr *sa)
{
    return &(((sockaddr_in*)sa)->sin_addr);
}

tcp_client::tcp_client(const std::string address, const std::string service, epoll_handler& handler):
    address_(address),
    service_(service),
    handler_(handler)
{
}

void tcp_client::connect()
{
    std::cout << address_ << " " << service_ << std::endl;
    addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address_.c_str(), service_.c_str(), &hints, &servinfo);
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

        if (::connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        throw tcp_exception("Failed to connect");
    }
    socket_ = new tcp_socket(socket_fd, handler_);

    freeaddrinfo(servinfo);
    std::cout << "on_connect: yay" << std::endl;
    on_connect(*socket_);
    handler_.add(socket_);
    socket_->connect_on_epoll([&](tcp_socket& socket)
        {
            std::cout << "on_epoll tcp_client eee" << std::endl;
            on_message(socket);
        });
}

void tcp_client::write(const std::string& data)
{
    socket_->write_all(data);
}
