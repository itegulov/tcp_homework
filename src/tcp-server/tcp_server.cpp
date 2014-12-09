#include "tcp_server.h"

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

        try
        {
            accepted_socket->make_non_blocking();
        }
        catch(const tcp_exception& e)
        {
            on_error(tcp_exception("couldn't make socket non blocking"));
            delete accepted_socket;
            continue;
        }

        accepted_socket->on_epoll.connect(boost::bind(&tcp_server::proceed_connection, this, _1));
        try {
            socket->server->handler->add(accepted_socket);
        }
        catch (...)
        {
            on_error(tcp_exception("couldn't add socket to epoll handler"));
            delete accepted_socket;
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

        if (!accepted_socket->is_open())
        {
            delete accepted_socket;
        }
    }
}

void tcp_server::proceed_connection(tcp_socket* socket)
{
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
