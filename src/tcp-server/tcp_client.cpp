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
    addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address_.c_str(), service_.c_str(), &hints, &servinfo);
    if (status != 0)
    {
        freeaddrinfo(servinfo);
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
    freeaddrinfo(servinfo);

    if (p == NULL) {
        throw tcp_exception("Failed to connect");
    }
    socket_ = std::shared_ptr<tcp_socket>(new tcp_socket(socket_fd, handler_));
    socket_->make_non_blocking();
    on_connect(*socket_);
    socket_->connect_on_epoll([&](tcp_socket& socket){
        std::exception_ptr eptr;
        try {
            socket.on_read(socket);
        }
        catch (...)
        {
            eptr = std::current_exception();
        }

        if (eptr != nullptr)
        {
            try {
                std::rethrow_exception(eptr);
            } catch(const std::exception& e) {
                on_error(e);
            }
        }
    });
    socket_->connect_on_read([&](tcp_socket& socket)
        {
            on_message(socket);
        });
    socket_->connect_on_close([&](tcp_socket& socket)
        {
            socket.handler_.stop();
        });
    handler_.add(socket_);
}

void tcp_client::write(const std::string& data)
{
    socket_->write_all(data);
}
