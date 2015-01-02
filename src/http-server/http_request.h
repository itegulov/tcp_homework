#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include "http_server_api.h"
#include "tcp_server_api.h"
#include "http_parser.h"
#include "http_response.h"

struct http_request
{
public:
    http_request(tcp_socket& socket, http_server& server);

    const std::string& get_method() const;
    const std::string& get_url() const;
    int get_major_version() const;
    int get_minor_version() const;
    const std::map<std::string, std::string>& get_headers() const;

private:
    void get_method(const std::string& method);
    void get_url(const std::string& url);
    void get_http_version(const int& major_version, const int& minor_version);
    void get_header(const std::string& name, const std::string& value);
    void headers_end();
    void get_body(const std::string& body);

    tcp_socket& socket_;
    http_server& server_;
    http_parser parser_;
    http_response response_;

    std::string method_;
    std::string url_;
    int major_version_;
    int minor_version_;
    std::map<std::string, std::string> headers_;
    std::string body_;
};

#endif // HTTP_REQUEST_H
