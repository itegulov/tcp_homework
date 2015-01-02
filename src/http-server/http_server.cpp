#include "http_server.h"
#include "tcp_server.h"

#include <iostream>

http_server::http_server(const std::string& address, const std::string& service, epoll_handler& handler)
    : tcp_server_(address, service, handler, 15)
{
    tcp_server_.connect_on_connection(boost::bind(&http_server::on_connection, this, _1));
}

std::vector<std::pair<http_request &, http_response &> > http_server::get_connections() const
{
    std::vector<std::pair<http_request &, http_response &> > vector;
    for (auto it = requests_.begin(); it != requests_.end(); it++)
    {
        vector.push_back(std::pair<http_request &, http_response &>(*(*it), (*it)->response_));
    }
    return vector;
}

void http_server::on_connection(tcp_socket& socket)
{
    std::cout << "New connection accepted!" << std::endl;
    http_request* request = new http_request(socket);
    request->connect_on_headers_end([this](http_request& request, http_response& response){
        on_request(request, response);
    });
    request->connect_on_body([this](http_request& request, const std::string& data, http_response& response){
        on_body(request, data, response);
    });
    requests_.push_back(std::unique_ptr<http_request>(request));

}
