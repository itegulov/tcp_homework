#include "tcp_socket.h"

tcp_socket::tcp_socket()
{
    fd_ = -1;
    is_open_ = false;
}

tcp_socket::tcp_socket(int fd)
{
    fd_ = fd;
    is_open_ = true;
}

tcp_socket::~tcp_socket()
{
    close_socket();
}

void tcp_socket::close_socket()
{
    if (is_open_)
    {
        is_open_ = false;
        close(fd_);
    }
}

int tcp_socket::get_socket_descriptor()
{
    if (!is_open_)
    {
        throw tcp_exception("Socket wasn't opened");
    }
    return fd_;
}

bool tcp_socket::is_open()
{
    return is_open_;
}

void tcp_socket::bind_socket(const char *address, const char *service)
{
    int status = 0;
    addrinfo hints;
    addrinfo* servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(address, service, &hints, &servinfo)) != 0)
    {
        throw tcp_exception(gai_strerror(status));
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(servinfo->ai_family, &(ipv4->sin_addr), ipstr, sizeof ipstr);
    int socket_fd = -1;
    addrinfo* rp;

    for (rp = servinfo; rp != nullptr; rp = rp->ai_next)
    {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)
        {
            continue;
        }

        status = bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);

        if (status == 0)
        {
            break;
        }

        close(socket_fd);
    }

    if (rp == nullptr)
    {
        throw tcp_exception("Couldn't bind to address");
    }

    freeaddrinfo(servinfo);
    fd_ = socket_fd;
    is_open_ = true;
}

void tcp_socket::make_socket_non_blocking()
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
    if (!is_open_)
    {
        throw tcp_exception("Socket wasn't opened");
    }
    int status = ::listen(fd_, max_pending_connections);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
    }
}

int tcp_socket::read_data(char *data, int max_size)
{
    if (!is_open_)
    {
        return 0;
    }
    int count = recv(fd_, data, max_size, 0);
    if (count == -1)
    {
        if (errno != EAGAIN)
        {
            close_socket();
            throw tcp_exception(strerror(errno));
        }
        return -1;
    }
    else if (count == 0)
    {
        close_socket();
        return 0;
    }
    return count;
}

int tcp_socket::write_data(const char *data, int max_size)
{
    if (!is_open_)
    {
        return 0;
    }
    int count = send(fd_, data, max_size, 0);
    if (count == -1)
    {
        throw tcp_exception(strerror(errno));
    }
    return count;
}

void tcp_socket::write_all(const char* data, int size)
{
    if (!is_open_)
    {
        return;
    }
    int total_count = 0;
    while (total_count != size)
    {
        int count = write_data(data, size - total_count);
        if (count <= 0)
        {
            break;
        }
        total_count += count;
    }
}

std::string tcp_socket::read_all()
{
    if (!is_open_)
    {
        return "";
    }
    std::string s = "";
    while (true) {
        char buffer[CHUNK_SIZE];
        if (read_data(buffer, CHUNK_SIZE) <= 0)
        {
            break;
        }
        else
        {
            s += buffer;
        }
    }
    return s;
}
