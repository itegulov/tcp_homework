#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "boost/signals2.hpp"
#include "http_server_api.h"

#include <iostream>
struct http_parser
{
    enum position {STARTING_LINE, HEAD, BODY};
public:
    http_parser();

    void parse(std::string data);

    template<typename T>
    void connect_on_method(T function)
    {
        on_method.connect(function);
    }

    template<typename T>
    void connect_on_url(T function)
    {
        on_url.connect(function);
    }

    template<typename T>
    void connect_on_http_version(T function)
    {
        on_http_version.connect(function);
    }

    template<typename T>
    void connect_on_header(T function)
    {
        on_header.connect(function);
    }

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
    void parse_starting_line(const std::string& starting_line);
    void parse_headers(const std::string& headers);

    boost::signals2::signal<void (const std::string&)> on_method;
    boost::signals2::signal<void (const std::string&)> on_url;
    boost::signals2::signal<void (const int&, const int&)> on_http_version;
    boost::signals2::signal<void (const std::string&, const std::string&)> on_header;
    boost::signals2::signal<void ()> on_headers_end;
    boost::signals2::signal<void (const std::string&)> on_body;
    position pos_ = STARTING_LINE;
    std::string headers_buffer_;
};

#endif // HTTP_PARSER_H
