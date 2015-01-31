#include "tcp_server.h"
#include "tcp_exception.h"

#include <iostream>

tcp_server::~tcp_server()
{

}

tcp_server::tcp_server(const std::string& address, const std::string& service, epoll_handler& handler, int max_pending_connections)
{
    std::unique_ptr<tcp_socket> main_socket(new tcp_socket(-1, handler));
    main_socket->bind(address, service);
    main_socket->make_non_blocking();
    main_socket->listen(max_pending_connections);
    main_socket->on_epoll.connect(boost::bind(&tcp_server::accept_connection, this, _1));
    std::cout << "BEFORE MOVE" << std::endl;
    handler.add(std::move(main_socket));
    std::cout << "AFTER MOVE" << std::endl;
}

void tcp_server::accept_connection(tcp_socket& socket)
{
    while (true)
    {
        sockaddr in_addr;
        socklen_t in_len = sizeof in_addr;

        int accepted_fd = accept (socket.get_descriptor(), &in_addr, &in_len);
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
        std::shared_ptr<tcp_socket> accepted_socket(new tcp_socket(accepted_fd, socket.handler_));
        try
        {
            accepted_socket->make_non_blocking();
        }
        catch(const tcp_exception& e)
        {
            //delete accepted_socket;
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

        std::exception_ptr eptr;
        try {
            on_connection(*accepted_socket);
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

        try {
            socket.handler_.add(std::move(accepted_socket));
        }
        catch (...)
        {
            //delete accepted_socket;
            on_error(tcp_exception("couldn't add socket to epoll handler"));
            continue;
        }
    }
}

void tcp_server::proceed_connection(tcp_socket& socket)
{
    std::exception_ptr eptr;
    try {
        socket.on_read(socket);
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
