#include "tcp_server.h"

tcp_server::~tcp_server()
{
    //TODO: implement
}

void tcp_server::start(const char * address, const char * service, epoll_handler* handler)
{
    main_socket = new tcp_socket();
    main_socket->bind(address, service);
    main_socket->make_non_blocking();
    main_socket->listen(max_pending_connections_);
    main_socket->on_epoll.connect(boost::bind(&tcp_server::accept_connection, this, _1, _2));
    handler->add(main_socket);
}

void tcp_server::accept_connection(tcp_socket* socket, epoll_handler* handler)
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
        tcp_socket* accepted_socket = new tcp_socket(accepted_fd);

        try
        {
            accepted_socket->make_non_blocking();
        }
        catch(tcp_exception)
        {
            continue;
        }

        accepted_socket->on_epoll.connect(boost::bind(&tcp_server::proceed_connection, this, _1, _2));
        try {
            handler->add(accepted_socket);
        }
        catch (...)
        {
            on_error(tcp_exception("couldn't add socket to epoll handler"));
            delete accepted_socket;
            continue;
        }

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
    }
}

void tcp_server::proceed_connection(tcp_socket* socket, epoll_handler* handler)
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

void tcp_server::set_max_pending_connections(int max)
{
    if (max < 0)
    {
        throw std::runtime_error("Max pending connections must be >= 0");
    }
    max_pending_connections_ = max;
}
