#include "epoll_handler.h"
#include <iostream>
//TODO: unique_ptr

epoll_handler::epoll_handler()
{
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
}

void epoll_handler::start()
{
    epoll_event events[MAX_EVENTS];
    memset(events, 0, sizeof events);
    while (true)
    {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        std::cout << "new events: " << n << std::endl;
        if (n == -1 && errno == EINTR)
        {
            break;
        }
        for (int i = 0; i < n; i++)
        {
            std::cout << "event fd: " << events[i].data.fd << std::endl;
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                std::unique_ptr<tcp_socket>& socket = sockets[events[i].data.fd];
                //tcp_socket& socket = sockets[events[i].data.fd];
                sockets.erase(socket->get_descriptor());
                continue;
            }
            else
            {
                //std::unique_ptr<tcp_socket> socket = sockets.find(events[i].data.fd)->second;
                std::unique_ptr<tcp_socket>& socket = sockets[events[i].data.fd];
                int fd = socket->get_descriptor();
                socket->on_epoll(*socket);
                if (!socket->is_open())
                {
                    std::cout << "erasing fd: " << fd << std::endl;
                    sockets.erase(fd);
                    //delete socket;
                }
            }
        }
    }
    ::close(epoll_fd_);
    //TODO: remove it
}

void epoll_handler::add(tcp_socket* socket)
{
    std::cout << "Adding to epoll_handler: " << socket->get_descriptor() << " " << std::endl;
    epoll_event event;
    memset(&event, 0, sizeof event);
    event.data.fd = socket->get_descriptor();
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->get_descriptor(), &event);
    if (status == -1)
    {
        throw std::runtime_error(strerror(errno));
    }
    sockets.insert(std::pair<int, std::unique_ptr<tcp_socket> >(socket->get_descriptor(), std::unique_ptr<tcp_socket>(socket)));
    std::cout << "added to epoll_handler: " << socket->get_descriptor() << " " << std::endl;
}

void epoll_handler::remove(const tcp_socket& socket)
{
    std::cout << "Removing tcp_socket" << std::endl;
    sockets.erase(socket.get_descriptor());
}
