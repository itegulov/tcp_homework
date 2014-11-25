#include "tcp_server.h"

tcp_server::~tcp_server()
{
    is_running_ = false;
    if (thread_ != nullptr && thread_->joinable())
    {
        const char * buf = "1";
        ssize_t count = event_socket_->write_data(buf, sizeof buf);
        if (count == -1)
        {
            thread_->detach();
        }
        else
        {
            thread_->join();
        }
    }
}

bool tcp_server::begin_listening(const char *address, const char * service)
{
    tcp_socket* socket = new tcp_socket();
    socket->bind_socket(address, service);
    socket->make_socket_non_blocking();
    socket->listen(max_pending_connections_);
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1)
    {
        socket->close_socket();
        throw tcp_exception(strerror(errno));
        return false;
    }

    epoll_event event;
    epoll_event *events;
    event.data.fd = socket->get_socket_descriptor();
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, socket->get_socket_descriptor(), &event);
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
    int event_fd_ = eventfd(0, 0);
    if (event_fd_ == -1)
    {
        throw tcp_exception(strerror(errno));
    }
    event_socket_ = new tcp_socket(event_fd_);
    event_socket_->make_socket_non_blocking();
    epoll_event event;
    event.data.fd = event_fd_;
    event.events = EPOLLIN | EPOLLET;
    int status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, event_socket_->get_socket_descriptor(), &event);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
    }
}

void tcp_server::run(tcp_socket* socket, epoll_event* events)
{
    epoll_event event;
    int status = 0;
    std::map<int, tcp_socket*> map;
    while (is_running_)
    {
        int n = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                map[events[i].data.fd]->close_socket();
                continue;
            }
            else if (socket->get_socket_descriptor() == events[i].data.fd)
            {
                //Notifiaction on main socket
                while (true)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len = sizeof in_addr;

                    int accepted_fd = accept (socket->get_socket_descriptor(), &in_addr, &in_len);
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
                                "(host=%s, port=%s)\n", accepted_socket->get_socket_descriptor(), hbuf, sbuf);
                        fflush(stdout);
                    }

                    try
                    {
                        accepted_socket->make_socket_non_blocking();
                    }
                    catch(...)
                    {
                        continue;
                    }

                    event.data.fd = accepted_socket->get_socket_descriptor();
                    event.events = EPOLLIN | EPOLLET;
                    status = epoll_ctl (epoll_fd_, EPOLL_CTL_ADD, accepted_socket->get_socket_descriptor(), &event);
                    if (status == -1)
                    {
                        perror ("epoll_ctl");
                        //TODO: check error
                        continue;
                    }
                    map[accepted_fd] = accepted_socket;
                    new_connection(accepted_socket);
                }
            }
            else if (event_socket_->get_socket_descriptor() == events[i].data.fd)
            {
                is_running_ = false;
                break;
            }
            else
            {
                //Read data
                map[events[i].data.fd]->on_read(map[events[i].data.fd]);
            }
        }
    }
    free(events);
    delete socket;
}

void tcp_server::stop_listening()
{
    is_running_ = false;
    thread_->join();
}

void tcp_server::set_max_pending_connections(int max)
{
    max_pending_connections_ = max;
}
