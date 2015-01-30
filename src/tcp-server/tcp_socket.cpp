#include "tcp_socket.h"
#include "epoll_handler.h"
#include "tcp_exception.h"

#include <iostream>

tcp_socket::tcp_socket(int fd, epoll_handler& handler): handler_(handler), fd_(fd), is_open_(fd != -1)
{
}

tcp_socket::tcp_socket(const tcp_socket &other): handler_(other.handler_), fd_(other.fd_), is_open_(other.is_open_)
{

}

tcp_socket::~tcp_socket()
{
    std::cout << "Deleting socket " << fd_ << std::endl;
    close();
    //server->handler->remove(this);
}

void tcp_socket::close()
{
    if (is_open())
    {
        assert (::close(fd_) == 0);
        on_close(*this);
        is_open_ = false;
    }
}

int tcp_socket::get_descriptor() const
{
    return fd_;
}

bool tcp_socket::is_open() const
{
    return is_open_;
}

void tcp_socket::bind(const std::string& address, const std::string& service)
{
    int status = 0;
    addrinfo hints;
    addrinfo* servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(address.c_str(), service.c_str(), &hints, &servinfo)) != 0)
    {
        throw tcp_exception(gai_strerror(status));
    }

    int socket_fd = -1;
    addrinfo* rp;

    for (rp = servinfo; rp != nullptr; rp = rp->ai_next)
    {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)
        {
            continue;
        }

        status = ::bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);

        if (status == 0)
        {
            break;
        }

        assert (::close(socket_fd) == 0);
    }

    if (rp == nullptr)
    {
        throw tcp_exception("Couldn't bind to address");
    }
    int option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, (char *)&option, sizeof option) < 0)
    {
        assert (::close(socket_fd) == 0);
        throw tcp_exception(strerror(errno));
    }

    freeaddrinfo(servinfo);
    fd_ = socket_fd;
    is_open_ = true;
}

void tcp_socket::make_non_blocking()
{
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1)
    {
        throw tcp_exception("fcntl() error");
    }
    flags |= O_NONBLOCK;
    int status = fcntl(fd_, F_SETFL, flags);
    if (status == -1)
    {
        throw tcp_exception("fcntl() error");
    }
}

void tcp_socket::listen(int max_pending_connections)
{
    if (!is_open())
    {
        throw tcp_exception("Socket wasn't opened");
    }
    int status = ::listen(fd_, max_pending_connections);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
    }
}

void tcp_socket::write_all(const std::string & data)
{
    if (!is_open())
    {
        return;
    }
    revert_flag(O_NONBLOCK);
    int written = write(fd_, data.c_str(), data.length());
    if (written == -1)
    {
        throw tcp_exception("couldn't send data");
    }
    revert_flag(O_NONBLOCK);
}

std::string tcp_socket::read_all()
{
    if (!is_open())
    {
        return "";
    }
    std::string ret;
    int len = 0;
    ioctl(fd_, FIONREAD, &len);
    if (len > 0) {
        char buf[len];
        len = read(fd_, buf, len);
        ret.append(buf, (size_t) len);
        return ret;
    } else return "";
}

void tcp_socket::revert_flag(int flag) {
    int flags = fcntl(fd_, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error(strerror(errno));
    }
    flags ^= flag;
    if (fcntl(fd_, F_SETFL, flags) == -1) {
        throw std::runtime_error(strerror(errno));
    }
}
