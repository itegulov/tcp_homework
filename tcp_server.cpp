#include "tcp_server.h"

tcp_server::~tcp_server()
{
    is_running_ = false;
    if (thread_ != nullptr && thread_->joinable())
    {
        ssize_t count = write(event_fd_, END_STR, sizeof END_STR);
        if (count == -1)
        {
            thread_->detach();
        }
        else
        {
            thread_->join();
        }
    }
    delete thread_;
}

bool tcp_server::listen(const char * address, const char * service)
{
    tcp_socket* socket = new tcp_socket();
    socket->bind(address, service);
    socket->make_non_blocking();
    socket->listen(max_pending_connections_);
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1)
    {
        socket->close();
        throw tcp_exception(strerror(errno));
        return false;
    }

    epoll_event event;
    epoll_event *events;
    memset(&event, 0, sizeof event);
    event.data.fd = socket->get_descriptor();
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->get_descriptor(), &event);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
        return false;
    }

    create_event_fd();

    //Buffer where events are returned
    events = (epoll_event*) calloc(MAX_EVENTS, sizeof event);
    is_running_ = true;
    thread_ = new std::thread(&tcp_server::run, this, socket, events);
    return true;
}

void tcp_server::create_event_fd()
{
    event_fd_ = eventfd(0, 0);
    if (event_fd_ == -1)
    {
        throw tcp_exception(strerror(errno));
    }
    int flags = fcntl(event_fd_, F_GETFL, 0);
    if (flags == -1)
    {
        throw tcp_exception("fcntl() error");
    }
    flags |= O_NONBLOCK;
    int status = fcntl(event_fd_, F_SETFL, flags);
    if (status == -1)
    {
        throw tcp_exception("fcntl() error");
    }
    epoll_event event;
    memset(&event, 0, sizeof event);
    event.data.fd = event_fd_;
    event.events = EPOLLIN | EPOLLET; //WUT?!
    status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, event_fd_, &event);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
    }
}

void tcp_server::run(tcp_socket* socket, epoll_event* events)
{
    epoll_event event;
    memset(&event, 0, sizeof event);
    int status = 0;
    std::map<int, tcp_socket*> sockets;
    while (is_running_)
    {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (n == -1)
        {
            perror("epoll_wait");
            //TODO: check error
            break;
        }
        for (int i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                sockets[events[i].data.fd]->close();
                continue;
            }
            else if (socket->get_descriptor() == events[i].data.fd)
            {
                //Notifiaction on main socket
                while (true)
                {
                    sockaddr in_addr;
                    socklen_t in_len = sizeof in_addr;

                    int accepted_fd = accept (socket->get_descriptor(), &in_addr, &in_len);
                    if (accepted_fd == -1)
                    {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK))
                        {
                            //No more connections
                            break;
                        }
                        else
                        {
                            perror("accept");
                            //TODO: check error
                            break;
                        }
                    }
                    tcp_socket* accepted_socket = new tcp_socket(accepted_fd);

                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    status = getnameinfo (&in_addr, in_len,
                                           hbuf, sizeof hbuf,
                                           sbuf, sizeof sbuf,
                                           NI_NUMERICHOST | NI_NUMERICSERV);
                    if (status == 0)
                    {
                        printf("Accepted connection on descriptor %d "
                                "(host=%s, port=%s)\n", accepted_socket->get_descriptor(), hbuf, sbuf);
                        fflush(stdout);
                    }

                    try
                    {
                        accepted_socket->make_non_blocking();
                    }
                    catch(...)
                    {
                        continue;
                    }

                    event.data.fd = accepted_socket->get_descriptor();
                    event.events = EPOLLIN | EPOLLET;
                    status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, accepted_socket->get_descriptor(), &event);
                    if (status == -1)
                    {
                        perror ("epoll_ctl");
                        //TODO: check error
                        continue;
                    }
                    sockets[accepted_fd] = accepted_socket;
                    new_connection(accepted_socket);
                    if (!accepted_socket->is_open())
                    {
                        sockets.erase(accepted_fd);
                        delete accepted_socket;
                    }
                }
            }
            else if (event_fd_ == events[i].data.fd)
            {
                //Signal to terminate has arrived
                is_running_ = false;
                break;
            }
            else
            {
                //Read data
                tcp_socket* socket = sockets[events[i].data.fd];
                socket->on_read(socket);
                if (!socket->is_open())
                {
                    sockets.erase(socket->get_descriptor());
                    delete socket;
                }
            }
        }
    }
    new_connection.disconnect_all_slots();
    for (auto socket : sockets)
    {
        delete socket.second;
    }
    free(events);
    close(epoll_fd_);
    close(event_fd_);
    delete socket;
}

void tcp_server::stop_listening()
{
    if (is_running_)
    {
        is_running_ = false;
        thread_->join();
    }
}

void tcp_server::set_max_pending_connections(int max)
{
    if (max < 0)
    {
        throw std::runtime_error("Max pending connections must be >= 0");
    }
    max_pending_connections_ = max;
}
