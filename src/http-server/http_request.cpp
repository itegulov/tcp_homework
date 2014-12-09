#include "http_request.h"

/***********
 * Getters *
 ***********/

http_request::http_method http_request::get_method() const
{
    return method_;
}

const std::string http_request::get_url() const
{
    return url_;
}

const std::string http_request::get_http_version() const
{
    return http_version_;
}

std::map<std::string, std::string> http_request::get_headers() const
{
    return headers_;
}

const std::string http_request::get_body() const
{
    //TODO: implement (do i need this?)
}

/***********
 * Setters *
 ***********/

void http_request::set_finished(bool finished)
{
    finished_ = finished;
}

void http_request::set_method(http_request::http_method method)
{
    method_ = method;
}

void http_request::set_http_version(std::string http_version)
{
    http_version_ = http_version;
}

void http_request::set_url(std::string url)
{
    url_ = url;
}

void http_request::set_headers(std::map<std::string, std::string> headers)
{
    headers_ = headers;
}
