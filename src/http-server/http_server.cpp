#include "http_server.h"
#include "http_connection.h"
#include "tcp_server.h"

#include <iostream>

http_server::http_server(const char *address, const char *service, epoll_handler* handler)
{
    tcp_server_ = new tcp_server(address, service, handler, 15);
    tcp_server_->connect_on_connection(boost::bind(&http_server::on_connection, this, _1));
}

http_server::~http_server()
{
    delete tcp_server_;
}

void http_server::on_connection(tcp_socket *socket)
{
    assert(tcp_server_ != nullptr);
    std::cout << "New connection accepted!" << std::endl;
    http_connection* connection = new http_connection(socket);
    connection->connect_new_request(boost::bind(&http_server::on_request, this, _1, _2));
}

void http_server::on_request(http_request *request, http_response *response)
{
    std::cout << "New request has came!" << std::endl;
    new_request(request, response);
}
