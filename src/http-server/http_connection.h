#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H
#include "tcp_socket.h"
#include "http_parser.h"
#include "http_server_api.h"

#include "boost/signals2.hpp"

#include <map>
#include <string>

class http_connection
{
public:
    http_connection(tcp_socket* socket);
    ~http_connection();

    void write(const char * data, size_t size);

    void connect_new_connection(void (*function)(http_request*, http_response*));
private:
    //Static callbacks
    static int message_begin(http_parser *parser);
    static int url(http_parser *parser, const char *at, size_t length);
    static int header_field(http_parser *parser, const char *at, size_t length);
    static int header_value(http_parser *parser, const char *at, size_t length);
    static int headers_complete(http_parser *parser);
    static int body(http_parser *parser, const char *at, size_t length);
    static int message_complete(http_parser *parser);

private:
    //Non-static callbacks
    void parse_request(tcp_socket* socket_);

private:
    boost::signals2::signal<void (http_request*, http_response*)> new_connection;

    tcp_socket* socket_;
    http_parser* parser_;
    http_parser_settings* parser_settings_;

    http_request* request_;
    std::string url_;
    std::map<std::string, std::string> headers_;

    std::string buffer_header_field_;
    std::string buffer_header_value_;

};

#endif // HTTP_CONNECTION_H
