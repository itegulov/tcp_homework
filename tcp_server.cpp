#include "tcp_server.h"

tcp_server::~tcp_server()
{
    running = false;
    if (t != nullptr && t->joinable())
    {
        char * buf = "1";
        int count = write(event_fd, buf, sizeof buf);
        t->join();
    }
}

bool tcp_server::begin_listening(char *address, char * service)
{
    int socket_fd = create_and_bind(address, service);
    if (socket_fd == -1)
    {
        throw tcp_exception(strerror(errno));
        return false;
    }
    int status = make_socket_non_blocking(socket_fd);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
        close(socket_fd);
        return false;
    }

    status = listen(socket_fd, max_pending_connections);

    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
        close(socket_fd);
        return false;
    }

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        throw tcp_exception(strerror(errno));
        close(socket_fd);
        return false;
    }

    epoll_event event;
    epoll_event *events;
    event.data.fd = socket_fd;
    event.events = EPOLLIN | EPOLLET;
    status = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event);
    if (status == -1)
    {
        throw tcp_exception(strerror(errno));
        return false;
    }
    //BEGIN
    event_fd = eventfd(0, 0);
    if (event_fd == -1)
    {
        throw tcp_exception(strerror(errno));
    }
    status = make_socket_non_blocking(event_fd);
    if (status == -1)
    {
        perror("non blocking");
        //TODO: check error
        return false;
    }
    event.data.fd = event_fd;
    event.events = EPOLLIN | EPOLLET;
    status = epoll_ctl (epoll_fd, EPOLL_CTL_ADD, event_fd, &event);
    if (status == -1)
    {
        perror ("epoll_ctl");
        //TODO: check error
        return false;
    }
    //END
    //Buffer where events are returned
    events = (epoll_event*) calloc(MAX_EVENTS, sizeof event);
    running = true;
    t = new std::thread(&tcp_server::run, this, socket_fd, events);
    return true;
}

void tcp_server::run(int socket_fd, epoll_event* events)
{
    epoll_event event;
    int status = 0;
    std::map<int, tcp_socket*> map;
    while (running)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++)
        {
            if (!running)
            {
                break;
            }
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                //Erorr occured
                fprintf (stderr, "epoll error\n");
                close (events[i].data.fd);
                continue;
            }
            else if (socket_fd == events[i].data.fd)
            {
                //Notifiaction on main socket
                while (true)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int accepted_fd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    accepted_fd = accept (socket_fd, &in_addr, &in_len);
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

                    status = getnameinfo (&in_addr, in_len,
                                           hbuf, sizeof hbuf,
                                           sbuf, sizeof sbuf,
                                           NI_NUMERICHOST | NI_NUMERICSERV);
                    if (status == 0)
                    {
                        printf("Accepted connection on descriptor %d "
                                "(host=%s, port=%s)\n", accepted_fd, hbuf, sbuf);
                        fflush(stdout);
                    }

                    status = make_socket_non_blocking(accepted_fd);
                    if (status == -1)
                    {
                        perror("non blocking");
                        //TODO: check error
                        continue;
                    }
                    event.data.fd = accepted_fd;
                    event.events = EPOLLIN | EPOLLET;
                    status = epoll_ctl (epoll_fd, EPOLL_CTL_ADD, accepted_fd, &event);
                    if (status == -1)
                    {
                        perror ("epoll_ctl");
                        //TODO: check error
                        continue;
                    }
                    tcp_socket* socket = new tcp_socket(accepted_fd);
                    map[accepted_fd] = socket;
                    new_connection(socket);
                }
            }
            else if (event_fd == events[i].data.fd)
            {
                break;
            }
            else
            {
                //Read data
                /*
                bool need_close = false;
                while (true)
                {
                    ssize_t count;
                    char buf[512];

                    count = read (events[i].data.fd, buf, sizeof buf);
                    if (count == -1)
                    {
                        if (errno != EAGAIN)
                        {
                            //If some weird error occurred
                            perror ("read");
                            //TODO: check eror
                            need_close = true;
                        }
                        break;
                    }
                    else if (count == 0)
                    {
                        //Reached end of stream (closed connection)
                        need_close = true;
                        break;
                    }
                    on_read(events[i].data.fd, buf, count);
                }

                if (need_close)
                {
                    close_connection(events[i].data.fd);
                    printf ("Closed connection on descriptor %d\n",
                            events[i].data.fd);
                    //Closing fd -> removing it from epoll fd
                    close (events[i].data.fd);
                }
                */
                map[events[i].data.fd]->on_read(map[events[i].data.fd]);
            }
        }
    }
    free(events);
    close(socket_fd);
}

void tcp_server::stop_listening()
{
    running = false;
    t->join();
}

int tcp_server::create_and_bind(char * address, char * service)
{
    int status = 0;
    addrinfo hints;
    addrinfo * servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(address, service, &hints, &servinfo)) != 0)
    {

        throw tcp_exception(gai_strerror(status));
        return -1;
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
        return -1;
    }

    freeaddrinfo(servinfo);
    return socket_fd;
}

int tcp_server::make_socket_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        throw tcp_exception("fcntl() error");
        return -1;
    }
    flags |= O_NONBLOCK;
    int status = fcntl(socket_fd, F_SETFL, flags);
    if (status == -1)
    {
        throw tcp_exception("fcntl() error");
        return -1;
    }
    return 0;
}

void tcp_server::set_max_pending_connections(int max)
{
    max_pending_connections = max;
}

/*
 * TCP Exception implementation
 */

const char* tcp_exception::what() const throw()
{
    return description;
}

tcp_exception::tcp_exception(const char * description)
{
    this->description = description;
}
