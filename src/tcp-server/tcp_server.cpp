#include "tcp_server.h"
#include <iostream>
tcp_server::~tcp_server()
{
    //TODO: implement
}

tcp_server::tcp_server(const char * address, const char * service, epoll_handler* handler, int max_pending_connections)
{
    tcp_socket* main_socket = new tcp_socket(this);
    main_socket->bind(address, service);
    main_socket->make_non_blocking();
    main_socket->listen(max_pending_connections);
    main_socket->on_epoll.connect(boost::bind(&tcp_server::accept_connection, this, _1));
    this->handler = handler;
    handler->add(main_socket);
}

void tcp_server::accept_connection(tcp_socket* socket)
{
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
        tcp_socket* accepted_socket = new tcp_socket(accepted_fd, socket->server);
        std::cout << "accepted: " << accepted_fd << " " << std::endl;
        try
        {
            accepted_socket->make_non_blocking();
        }
        catch(const tcp_exception& e)
        {
            delete accepted_socket;
            on_error(tcp_exception("couldn't make socket non blocking"));
            continue;
        }

        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

        int status = getnameinfo (&in_addr, in_len,
            hbuf, sizeof hbuf,
            sbuf, sizeof sbuf,
            NI_NUMERICHOST | NI_NUMERICSERV);

        if (status == 0)
        {
            printf("Accepted connection on descriptor %d "
            "(host=%s, port=%s)\n", accepted_socket->get_descriptor(), hbuf, sbuf);
            fflush(stdout);
        }

        accepted_socket->on_epoll.connect(boost::bind(&tcp_server::proceed_connection, this, _1));
        std::cout << "on_epoll connected: " << accepted_fd << " " << std::endl;
        try {
            socket->server->handler->add(accepted_socket);
        }
        catch (...)
        {
            delete accepted_socket;
            on_error(tcp_exception("couldn't add socket to epoll handler"));
            continue;
        }

        std::exception_ptr eptr;
        try {
            on_connection(accepted_socket);
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
        std::cout << "success: " << accepted_fd << " " << std::endl;
    }
}

void tcp_server::proceed_connection(tcp_socket* socket)
{
    std::cout << "on_epoll: proceed_connection enter" << std::endl;
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
}
