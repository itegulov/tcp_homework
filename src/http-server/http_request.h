#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include "http_server_api.h"
#include "tcp_server_api.h"
#include "http_parser.h"
#include "http_response.h"

struct http_request
{
    friend struct http_server;
public:
    http_request(tcp_socket& socket);

    const std::string& get_method() const;
    const std::string& get_url() const;
    int get_major_version() const;
    int get_minor_version() const;
    const std::map<std::string, std::string>& get_headers() const;

    template<typename T>
    void connect_on_headers_end(T function)
    {
        on_headers_end.connect(function);
    }

    template<typename T>
    void connect_on_body(T function)
    {
        on_body.connect(function);
    }

private:

    boost::signals2::signal<void (http_request&, http_response&)> on_headers_end;
    boost::signals2::signal<void (http_request&, const std::string&, http_response&)> on_body;

    tcp_socket& socket_;
    http_parser parser_;
    http_response response_;

    std::string method_;
    std::string url_;
    int major_version_;
    int minor_version_;
    std::map<std::string, std::string> headers_;
};

struct http_client_request {
public:
    http_client_request(tcp_socket& socket);

    const std::string& get_code() const;
    const std::string& get_code_info() const;
    int get_major_version() const;
    int get_minor_version() const;
    const std::map<std::string, std::string>& get_headers() const;

    template<typename T>
    void connect_on_headers_end(T function)
    {
        on_headers_end.connect(function);
    }

    template<typename T>
    void connect_on_body(T function)
    {
        on_body.connect(function);
    }

private:

    boost::signals2::signal<void (http_client_request&, http_response&)> on_headers_end;
    boost::signals2::signal<void (http_client_request&, const std::string&, http_response&)> on_body;

    tcp_socket& socket_;
    http_client_parser parser_;
    http_response response_;

    std::string code_;
    std::string code_info_;
    int major_version_;
    int minor_version_;
    std::map<std::string, std::string> headers_;
};

#endif // HTTP_REQUEST_H
