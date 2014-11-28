#include "epoll_handler.h"

epoll_handler::epoll_handler()
{
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    create_event_fd();
}

epoll_handler::~epoll_handler()
{

}

void epoll_handler::start()
{
    events = (epoll_event*) calloc(MAX_EVENTS, sizeof (epoll_event));
    while (true)
    {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (n == -1 && errno == EINTR)
        {
            break;
        }
        for (int i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                tcp_socket* socket = sockets[events[i].data.fd];
                sockets.erase(socket->get_descriptor());
                delete socket;
                continue;
            }
            else if (events[i].data.fd == event_fd_)
            {
                break;
            }
            else
            {
                tcp_socket* socket = sockets[events[i].data.fd];
                socket->on_epoll(socket, this);
                if (!socket->is_open())
                {
                    sockets.erase(events[i].data.fd);
                    delete socket;
                }
            }
        }
    }
    ::close(epoll_fd_);
    for (auto socket : sockets)
    {
        delete socket.second;
    }
    free(events);
}

void epoll_handler::stop()
{
    ssize_t count = -1;
    while (count == -1)
    {
        count = write(event_fd_, END_STR, sizeof END_STR);
    }
}

void epoll_handler::add(tcp_socket* socket)
{
    epoll_event event;
    memset(&event, 0, sizeof event);
    event.data.fd = socket->get_descriptor();
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, socket->get_descriptor(), &event);
    if (status == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    sockets[socket->get_descriptor()] = socket;
    printf("Added %d\n", socket->get_descriptor());
    fflush(stdout);
}

void epoll_handler::create_event_fd()
{
    event_fd_ = eventfd(0, 0);
    if (event_fd_ == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    int flags = fcntl(event_fd_, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("fcntl() error");
    }
    flags |= O_NONBLOCK;
    int status = fcntl(event_fd_, F_SETFL, flags);
    if (status == -1)
    {
        throw std::runtime_error("fcntl() error");
    }
    add(new tcp_socket(event_fd_));
}
