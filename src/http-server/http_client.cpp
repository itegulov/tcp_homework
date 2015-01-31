#include "http_client.h"
#include "http_request.h"
#include "http_response.h"

http_client::http_client(const std::string &address, const std::string &service, const std::string &method,
                         const std::string &url, const std::string& headers, epoll_handler &handler):
    method_(method),
    url_(url),
    headers_(headers),
    client_(address, service, handler)
{
    client_.connect_on_connect([&](tcp_socket& socket){
        on_connect(socket);
        socket.write_all(method_ + " " + url_ + " HTTP/1.0\nHost: " + domain_ + "\n" + headers_ + "\r\n\r\n");
        request_ = std::unique_ptr<http_client_request>(new http_client_request(socket));
        request_->connect_on_headers_end([&](http_client_request& request, http_response& response){
            on_response(request, response);
        });
        request_->connect_on_body([this](http_client_request& request, const std::string& data, http_response& response){
            on_body(request, data, response);
        });
    });
}

http_client::~http_client()
{
}

void http_client::connect()
{
    client_.connect();
}

void http_client::write(const std::string &data)
{
    client_.write(data);
}
