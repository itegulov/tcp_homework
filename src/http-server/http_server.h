#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H
#include "http_server_api.h"
#include "tcp_server.h"
#include "epoll_handler.h"

#include "boost/signals2.hpp"

struct http_server
{
public:
    http_server(const std::string& address, const std::string& service, epoll_handler& handler);

    template<typename T>
    void connect_new_request(T function)
    {
        new_request.connect(function);
    }

private:
    boost::signals2::signal<void (http_request*, http_response*)> new_request;

    void on_connection(tcp_socket* socket);
    void on_request(http_request* request, http_response* response);

    tcp_server tcp_server_;
};

#endif // HTTP_SERVER_H
