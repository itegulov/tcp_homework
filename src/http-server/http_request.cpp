#include "http_request.h"
#include "http_server.h"
#include "tcp_socket.h"

#include "boost/bind.hpp"

//HTTP_SERVER_REQUEST

http_request::http_request(tcp_socket &socket):
    socket_(socket),
    response_(socket)
{
    parser_.connect_on_method([this](const std::string& method){
        method_ = method;
    });
    parser_.connect_on_url([this](const std::string& url){
        url_ = url;
    });
    parser_.connect_on_http_version([this](int major_version, int minor_version){
        major_version_ = major_version;
        minor_version_ = minor_version;
    });
    parser_.connect_on_header([this](const std::string& name, const std::string& value){
        headers_[name] = value;
    });
    parser_.connect_on_headers_end([this](){
        on_headers_end(*this, response_);
    });
    parser_.connect_on_body([this](const std::string& body){
        on_body(*this, body, response_);
    });
    socket_.connect_on_read([&](tcp_socket& socket) {
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

//HTTP_CLIENT_REQUEST

http_client_request::http_client_request(tcp_socket &socket):
    socket_(socket),
    response_(socket)
{
    parser_.connect_on_code([this](const std::string& code){
        code_ = code;
    });
    parser_.connect_on_code_info([this](const std::string& code_info){
        code_info_ = code_info;
    });
    parser_.connect_on_http_version([this](int major_version, int minor_version){
        major_version_ = major_version;
        minor_version_ = minor_version;
    });
    parser_.connect_on_header([this](const std::string& name, const std::string& value){
        headers_[name] = value;
    });
    parser_.connect_on_headers_end([this](){
        on_headers_end(*this, response_);
    });
    parser_.connect_on_body([this](const std::string& body){
        on_body(*this, body, response_);
    });
    socket_.connect_on_read([&](tcp_socket& socket) {
        std::string data = socket.read_all();
        parser_.parse(data);
    });
}

const std::string &http_client_request::get_code() const
{
    return code_;
}

const std::string &http_client_request::get_code_info() const
{
    return code_info_;
}

int http_client_request::get_major_version() const
{
    return major_version_;
}

int http_client_request::get_minor_version() const
{
    return minor_version_;
}

const std::map<std::string, std::string> &http_client_request::get_headers() const
{
    return headers_;
}
