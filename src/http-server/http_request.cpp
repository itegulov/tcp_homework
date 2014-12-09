#include "http_request.h"


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
