#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "http_server_api.h"
#include "tcp_server.h"
#include "epoll_handler.h"
#include "http_request.h"

#include <functional>
#include <vector>
#include <iostream>

#include "boost/signals2.hpp"
struct http_server
{
    friend struct http_request;
public:
    http_server(const std::string& address, const std::string& service, epoll_handler& handler);

    template<typename T>
    void connect_on_request(T function)
    {
        on_request.connect(function);
    }

private:
    boost::signals2::signal<void (http_request&, http_response&)> on_request;

    void on_connection(tcp_socket& socket);

    tcp_server tcp_server_;
    std::vector<std::unique_ptr<http_request> > requests_;
};

#endif // HTTP_SERVER_H
