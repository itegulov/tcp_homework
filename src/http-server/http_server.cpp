#include "http_server.h"
#include "tcp_server.h"

#include <iostream>

http_server::http_server(const std::string& address, const std::string& service, epoll_handler& handler)
    : tcp_server_(address, service, handler, 15)
{
    tcp_server_.connect_on_connection(boost::bind(&http_server::on_connection, this, _1));
}

void http_server::on_connection(tcp_socket& socket)
{
    std::cout << "New connection accepted!" << std::endl;
    requests_.push_back(std::unique_ptr<http_request>(new http_request(socket, *this)));
}
