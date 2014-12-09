#include "http_connection.h"
#include "http_request.h"
#include "http_response.h"

http_connection::http_connection(tcp_socket *socket):
    socket_(socket)
{
    parser_ = (http_parser *)malloc(sizeof(http_parser));
    http_parser_init(parser_, HTTP_REQUEST);

    parser_settings_ = new http_parser_settings();
    parser_settings_->on_message_begin = message_begin;
    parser_settings_->on_url = url;
    parser_settings_->on_header_field = header_field;
    parser_settings_->on_header_value = header_value;
    parser_settings_->on_headers_complete = headers_complete;
    parser_settings_->on_body = body;
    parser_settings_->on_message_complete = message_complete;

    parser_->data = this;

    socket_->connect_on_read(boost::bind(&http_connection::parse_request, this, _1));
}

http_connection::~http_connection()
{
    delete socket_;
    socket_ = nullptr;

    free(parser_);
    parser_ = nullptr;

    delete parser_settings_;
    parser_settings_ = nullptr;
}

void http_connection::write(const char *data, size_t size)
{
    socket_->write_all(data, size);
}

/********************
 * Static Callbacks *
 *******************/

int http_connection::message_begin(http_parser *parser)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    connection->headers_.clear();
    connection->url_ = "";
    connection->request_ = new http_request(connection);
    return 0;
}

int http_connection::url(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    connection->url_.append(at, length);
    return 0;
}

int http_connection::header_field(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    if (connection->buffer_header_field_ != "" && connection->buffer_header_value_ != "")
    {
        connection->headers_[connection->buffer_header_field_] = connection->buffer_header_value_;

        connection->buffer_header_field_ = "";
        connection->buffer_header_value_ = "";
    }
    connection->buffer_header_field_ += std::string(at, length);
    return 0;
}

int http_connection::header_value(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    connection->buffer_header_value_ += std::string(at, length);
    return 0;
}

int http_connection::headers_complete(http_parser *parser)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    assert(parser->method <= 4);
    connection->request_->set_method(static_cast<http_request::http_method>(parser->method));
    connection->request_->set_http_version(parser->http_major + "." + parser->http_minor);

    http_parser_url url_info;
    int r = http_parser_parse_url(connection->url_.c_str(), sizeof connection->url_.c_str(), parser->method == HTTP_CONNECT, &url_info);
    assert(r == 0);

    connection->request_->set_url(connection->url_);

    connection->headers_[connection->buffer_header_field_] = connection->buffer_header_value_; //Last element left
    connection->request_->set_headers(connection->headers_);

    //TODO: some more info to request?

    //TODO: some more slots?

    connection->new_connection(connection->request_, new http_response(connection));
    return 0;
}

int http_connection::body(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    connection->request_->on_data(at, length);
    return 0;
}

int http_connection::message_complete(http_parser *parser)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    connection->request_->set_finished(true);
    connection->request_->on_end();
    return 0;
}

/************************
 * Non-static Callbacks *
 ************************/

void http_connection::parse_request(tcp_socket *socket_)
{
    assert(parser_ != nullptr);

    const char * data = socket_->read_all();
    http_parser_execute(parser_, parser_settings_, data, strlen(data));
}