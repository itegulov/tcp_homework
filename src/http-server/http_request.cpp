#include "http_request.h"
#include "http_server.h"
#include "tcp_socket.h"

#include "boost/bind.hpp"

http_request::http_request(tcp_socket &socket, http_server& server):
    socket_(socket),
    server_(server)
{
    parser_.connect_on_method(boost::bind(&http_request::get_method, this, _1));
    parser_.connect_on_url(boost::bind(&http_request::get_url, this, _1));
    parser_.connect_on_http_version(boost::bind(&http_request::get_http_version, this, _1, _2));
    parser_.connect_on_header(boost::bind(&http_request::get_header, this, _1, _2));
    parser_.connect_on_headers_end(boost::bind(&http_request::headers_end, this));
    parser_.connect_on_body(boost::bind(&http_request::get_body, this, _1));
    socket_.connect_on_read([&](tcp_socket& socket){
        std::string data = socket.read_all();
        parser_.parse(data);
    });
}

const std::string &http_request::get_method() const
{
    return method_;
}

const std::string &http_request::get_url() const
{
    return url_;
}

int http_request::get_major_version() const
{
    return major_version_;
}

int http_request::get_minor_version() const
{
    return minor_version_;
}

const std::map<std::string, std::string> &http_request::get_headers() const
{
    return headers_;
}

void http_request::get_method(const std::string &method)
{
    method_ = method;
}

void http_request::get_url(const std::string &url)
{
    url_ = url;
}

void http_request::get_http_version(const int &major_version, const int &minor_version)
{
    major_version_ = major_version;
    minor_version_ = minor_version;
}

void http_request::get_header(const std::string &name, const std::string &value)
{
    headers_[name] = value;
}

void http_request::headers_end()
{
    std::cout << "Headers end" << std::endl;
    server_.on_request(*this, response_);
}

void http_request::get_body(const std::string &body)
{
    body_ += body;
}
