#include "http_connection.h"
#include "http_request.h"
#include "http_response.h"

#include <iostream>
void f(http_connection* connection);

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
    socket_->connect_on_read(boost::bind(&f, this));
}

void f(http_connection* connection) {
    std::cout << "magic function" << std::endl;
    if (connection->need_to_delete())
    {
        delete connection;
    }
}

http_connection::~http_connection()
{
    headers_.clear();
    std::cout << "Deleting http_connection" << std::endl;
    socket_->close();

    free(parser_);
    parser_ = nullptr;

    delete parser_settings_;
    parser_settings_ = nullptr;

    //delete request_;
    //request_ = nullptr;

    delete response_;
    response_ = nullptr;

    std::cout << "End deleting http_connection" << std::endl;
}

void http_connection::write(const char *data, size_t size)
{
    std::cout << "Writing: " << std::string(data, size) << std::endl;
    socket_->write_all(data, size);
}

bool http_connection::need_to_delete()
{
    return deleting_;
}

/********************
 * Static Callbacks *
 *******************/

int http_connection::message_begin(http_parser *parser)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    connection->headers_.clear();
    connection->url_ = "";
    connection->request_ = new http_request();
    return 0;
}

int http_connection::url(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    std::cout << "New url: " << std::string(at, length) << std::endl;
    connection->url_.append(at, length);
    return 0;
}

int http_connection::header_field(http_parser *parser, const char *at, size_t length)
{
    std::cout << "begin new header field" << std::endl;
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    if (connection->buffer_header_field_ != "" && connection->buffer_header_value_ != "")
    {
        connection->headers_[connection->buffer_header_field_] = connection->buffer_header_value_;

        connection->buffer_header_field_ = "";
        connection->buffer_header_value_ = "";
    }
    std::string s(at, length);
    std::cout << "New header field: " << s << std::endl;
    std::cout << "Mda ser: " << connection->buffer_header_field_ << std::endl;
    connection->buffer_header_field_ = s;
    std::cout << "Norm tema field" << std::endl;
    return 0;
}

int http_connection::header_value(http_parser *parser, const char *at, size_t length)
{
    std::cout << "begin new header value" << std::endl;
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    std::cout << "New header value: " << std::string(at, length) << std::endl;
    connection->buffer_header_value_ += std::string(at, length);
    return 0;
}

int http_connection::headers_complete(http_parser *parser)
{
    std::cout << "Going to complete headers" << std::endl;
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);
    assert(parser->method <= 4);

    connection->request_->set_method(static_cast<http_request::http_method>(parser->method));
    char buffer[10];
    sprintf(buffer, "%d.%d", parser->http_major, parser->http_minor);
    connection->request_->set_http_version(std::string(buffer));
    std::cout << "Proccessing headrs..." << std::endl;

    http_parser_url url_info;
    int r = http_parser_parse_url(connection->url_.c_str(), connection->url_.length(), parser->method == HTTP_CONNECT, &url_info);
    assert(r == 0);

    connection->request_->set_url(connection->url_);

    std::cout << "URL OK, ser..." << std::endl;
    std::cout << "Mdaaaaaa " << connection->buffer_header_field_ << std::endl;
    if (connection->buffer_header_field_ != "")
    {
        connection->headers_[connection->buffer_header_field_] = connection->buffer_header_value_; //Last element left
    }
    connection->request_->set_headers(connection->headers_);

    std::cout << "MAP OK, ser..." << std::endl;

    //TODO: some more info to request?

    //TODO: some more slots?

    connection->response_ = new http_response(connection);
    connection->response_->on_done.connect(boost::bind(&http_connection::on_done, connection, _1));

    std::cout << "Headers has been completed" << std::endl;

    connection->new_request(connection->request_, connection->response_);
    return 0;
}

int http_connection::body(http_parser *parser, const char *at, size_t length)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);
    std::cout << "Body: " << std::string(at, length) << std::endl;
    connection->request_->on_data(at, length);
    return 0;
}

int http_connection::message_complete(http_parser *parser)
{
    http_connection* connection = static_cast<http_connection*>(parser->data);
    assert(connection->request_ != nullptr);

    std::cout << "Message completed" << std::endl;
    connection->request_->set_finished(true);
    connection->request_->on_end();
    //connection->deleting_ = true;
    return 0;
}

/************************
 * Non-static Callbacks *
 ************************/

void http_connection::parse_request(tcp_socket *socket_)
{
    std::cout << "Parse begin" << std::endl;
    assert(parser_ != nullptr);
    std::string s = socket_->read_all();
    std::cout << s << std::endl;
    const char * data = s.c_str();
    http_parser_execute(parser_, parser_settings_, data, strlen(data));
}

void http_connection::on_done(http_response *response)
{
    std::cout << "Done with responding" << std::endl;
    //socket_->close();
}
