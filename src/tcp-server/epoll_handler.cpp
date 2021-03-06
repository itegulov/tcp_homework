#include "epoll_handler.h"
#include "tcp_socket.h"

#include <iostream>
#include <cassert>

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
    assert(sockets.size() == 0);
}

void epoll_handler::start()
{
    epoll_event events[MAX_EVENTS];
    memset(events, 0, sizeof events);
    running_ = true;
    while (running_)
    {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        std::cout << "Got " << n << " events" << std::endl;
        if (n == -1 && errno == EINTR)
        {
            sockets.clear();
            running_ = false;
            break;
        }
        for (int i = 0; i < n; i++)
        {
            std::cout << "Got event: " << events[i].events << std::endl;
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                std::cout << "Error occured" << std::endl;
                std::shared_ptr<tcp_socket>& socket = sockets[events[i].data.fd];
                std::cout << "Error on socket: " << socket->get_descriptor() << std::endl;
                on_error(*socket);
                sockets.erase(socket->get_descriptor());
                continue;
            }
            else if (events[i].data.fd == event_fd_)
            {
                sockets.clear();
                running_ = false;
                break;
            }
            else
            {
                std::shared_ptr<tcp_socket>& socket = sockets[events[i].data.fd];
                socket->on_epoll(*socket);
                if (!socket->is_open())
                {
                    sockets.erase(socket->get_descriptor());
                }
            }
        }
    }
    ::close(epoll_fd_);
    ::close(event_fd_);
}

void epoll_handler::stop()
{
    eventfd_write(event_fd_, 1);
}

void epoll_handler::add(std::shared_ptr<tcp_socket> socket)
{
    epoll_event event;
    memset(&event, 0, sizeof event);
    event.data.fd = socket->get_descriptor();
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->get_descriptor(), &event);
    if (status == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    sockets.insert(std::pair<int, std::shared_ptr<tcp_socket> >(socket->get_descriptor(), socket));
}

void epoll_handler::create_event_fd()
{
    event_fd_ = eventfd(0, EFD_NONBLOCK);
    if (event_fd_ == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    epoll_event event = {0};
    event.data.fd = event_fd_;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event) == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
}
