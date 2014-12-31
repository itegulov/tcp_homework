#include "http_response.h"
#include "http_connection.h"
#include "tcp_socket.h"

#include <exception>

void http_response::set_header(const std::string &field, const std::string value)
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
    char buffer[50];
    int res = sprintf(buffer, "HTTP/1.1 %d %s\r\n", status, "code"); //TODO: fix code to code representation
    connection_->write(std::string(buffer));
    write_headers();
    connection_->write("\r\n");
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

    connection_->write(data);
}

void http_response::done()
{
    closed_ = true;
    on_done(this);
}

http_response::http_response(http_connection *connection)
{
    connection_ = connection;
}

void http_response::write_header(std::string field, std::string value)
{
    if (closed_)
    {
        throw std::runtime_error("http_response::write_header() can't write header after response is finished");
    }
    connection_->write(field);
    connection_->write(": ");
    connection_->write(value);
    connection_->write("\r\n");
}

void http_response::write_headers()
{
    for(auto item : headers_)
    {
        write_header(item.first, item.second);
    }
}
