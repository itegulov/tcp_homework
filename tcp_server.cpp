#include "tcp_server.h"

tcp_server::~tcp_server()
{
    is_running_ = false;
    if (thread_ != nullptr && thread_->joinable())
    {
        ssize_t count = write(event_fd_, END_STR, sizeof END_STR);
        if (count > 0) {
            thread_->join();
        }
    }
    delete thread_;
}

void tcp_server::start(const char * address, const char * service)
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
    }

    create_event_fd();

    events = (epoll_event*) calloc(MAX_EVENTS, sizeof event);
    is_running_ = true;
    thread_ = new std::thread(&tcp_server::run, this, socket, events);
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
    event.events = EPOLLIN | EPOLLET;
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
                            //Error occured
                            on_error(tcp_exception(strerror(errno)));
                            break;
                        }
                    }
                    tcp_socket* accepted_socket = new tcp_socket(accepted_fd);

                    try
                    {
                        accepted_socket->make_non_blocking();
                    }
                    catch(tcp_exception)
                    {
                        continue;
                    }

                    event.data.fd = accepted_socket->get_descriptor();
                    event.events = EPOLLIN | EPOLLET;
                    status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, accepted_socket->get_descriptor(), &event);

                    if (status == -1)
                    {
                        //Couldn't add to epoll
                        on_error(tcp_exception(strerror(errno)));
                        delete accepted_socket;
                        continue;
                    }

                    sockets[accepted_fd] = accepted_socket;
                    socket_deleted = false;
                    std::exception_ptr eptr;
                    try {
                        new_connection(accepted_socket);
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

                    if (socket_deleted)
                    {
                        sockets.erase(accepted_fd);
                    }
                    else
                    {
                        if (!accepted_socket->is_open())
                        {
                            sockets.erase(accepted_fd);
                            delete accepted_socket;
                        }
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
                int cur_fd = socket->get_descriptor();
                socket_deleted = false;
                std::exception_ptr eptr;
                try {
                    socket->on_read(socket);
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
                if (socket_deleted)
                {
                    sockets.erase(cur_fd);
                }
                else
                {
                    if (!socket->is_open())
                    {
                        sockets.erase(socket->get_descriptor());
                        delete socket;
                    }
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

void tcp_server::stop()
{
    if (is_running_)
    {
        is_running_ = false;
        if (thread_->joinable()) {
            ssize_t count = write(event_fd_, END_STR, sizeof END_STR);
            if (count > 0) {
                thread_->join();
            }
        }
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
