#include "http_response.h"
#include "tcp_socket.h"
#include <string>
#include <iostream>
#include <sstream>

http_response::http_response(tcp_socket &socket):
    socket_(socket)
{

}

void http_response::set_header(const std::string& field, const std::string& value)
{
    if (!closed_)
    {
        headers_[field] = value;
    }
    else
    {
        throw std::runtime_error("http_response::set_header() can't set header after response is finished");
    }
}

void http_response::write_head(http_response::status_code code)
{
    if (closed_)
    {
        throw std::runtime_error("http_response::write_head() can't write head after response is finished");
    }
    if (header_writen_)
    {
        throw std::runtime_error("http_response::write_head() can't write head more than one time");
    }
    int status = static_cast<int>(code);
    std::ostringstream string_stream;
    string_stream << "HTTP/1.1 " << status << " " << "code" << "\r\n";
    socket_.write_all(string_stream.str());
    write_headers();
    socket_.write_all("\r\n");
    header_writen_ = true;
}

void http_response::write(const std::string& data)
{
    if (closed_)
    {
        throw std::runtime_error("http_response::write_head() can't write head after response is finished");
    }
    if (!header_writen_)
    {
        throw std::runtime_error("http_response::write_head() can't write head before header code is written");
    }

    socket_.write_all(data);
}

void http_response::done()
{
    closed_ = true;
    socket_.close();
    on_done(this);
}

void http_response::write_header(const std::string& field, const std::string& value)
{
    if (closed_)
    {
        throw std::runtime_error("http_response::write_header() can't write header after response is finished");
    }
    socket_.write_all(field);
    socket_.write_all(": ");
    socket_.write_all(value);
    socket_.write_all("\r\n");
}

void http_response::write_headers()
{
    for(auto item : headers_)
    {
        write_header(item.first, item.second);
    }
}
