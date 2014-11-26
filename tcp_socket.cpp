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
    close();
}

void tcp_socket::close()
{
    if (is_open_)
    {
        ::close(fd_);
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

void tcp_socket::bind(const char *address, const char *service)
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

    sockaddr_in *ipv4 = (sockaddr_in *) servinfo->ai_addr;
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

        status = ::bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);

        if (status == 0)
        {
            break;
        }

        ::close(socket_fd);
    }

    if (rp == nullptr)
    {
        throw tcp_exception("Couldn't bind to address");
    }
    int option = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, (char *)&option, sizeof option) < 0)
    {
        ::close(socket_fd);
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

ssize_t tcp_socket::read_data(char *data, ssize_t max_size)
{
    if (!is_open_)
    {
        return 0;
    }
    ssize_t count = recv(fd_, data, max_size, 0);
    if (count == -1)
    {
        if (errno != EAGAIN)
        {
            close();
            throw tcp_exception(strerror(errno));
        }
        return -1;
    }
    else if (count == 0)
    {
        close();
        return 0;
    }
    return count;
}

ssize_t tcp_socket::write_data(const char *data, ssize_t max_size) const
{
    if (!is_open_)
    {
        return 0;
    }
    ssize_t count = send(fd_, data, max_size, 0);
    if (count == -1)
    {
        throw tcp_exception(strerror(errno));
    }
    return count;
}

void tcp_socket::write_all(const char* data, ssize_t size) const
{
    if (!is_open_)
    {
        return;
    }
    ssize_t total_count = 0;
    while (total_count != size)
    {
        ssize_t count = write_data(data, size - total_count);
        if (count <= 0)
        {
            break;
        }
        data += count;
        total_count += count;
    }
}

char* tcp_socket::read_all()
{
    if (!is_open_)
    {
        char * empty = new char[1];
        empty[0] = '\0';
        return empty;
    }
    ssize_t total_count = 0;
    char* result = new char[RESULT_SIZE + 1];
    strcpy(result, "");
    char buffer[CHUNK_SIZE];
    while (total_count < RESULT_SIZE) {
        memset(buffer, 0, CHUNK_SIZE);
        ssize_t count = read_data(buffer, std::min(CHUNK_SIZE, RESULT_SIZE - total_count));
        if (count <= 0)
        {
            break;
        }
        else
        {
            strcat(result, buffer);
            total_count += count;
        }
    }
    return result;
}
