#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "tcp_client.h"
#include "http_request.h"
#include "http_response.h"

struct http_client
{
public:

    http_client(const std::string &address, const std::string &service, const std::string &method,
                const std::string &url, const std::string& headers, epoll_handler &handler);
    ~http_client();

    void connect();
    void write(const std::string& data);

    template<typename T>
    void connect_on_response(T function)
    {
        on_response.connect(function);
    }

    template<typename T>
    void connect_on_connect(T function)
    {
        on_connect.connect(function);
    }

    template<typename T>
    void connect_on_body(T function)
    {
        on_body.connect(function);
    }

    void set_domain(const std::string& domain)
    {
        domain_ = domain;
    }

private:
    boost::signals2::signal<void (http_client_request&, http_response&)> on_response;
    boost::signals2::signal<void (tcp_socket&)> on_connect;
    boost::signals2::signal<void (http_client_request&, const std::string&, http_response&)> on_body;

    std::string method_;
    std::string url_;
    std::string domain_;
    std::string headers_;

    tcp_client client_;
    http_client_request* request_ = nullptr;
};

#endif // HTTP_CLIENT_H
