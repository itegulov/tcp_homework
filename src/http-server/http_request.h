#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H
#include <map>
#include <string>

#include "boost/signals2.hpp"

#include "http_server_api.h"

class http_request
{
    friend struct http_connection;
public:
    http_request(http_connection* connection);

    enum http_method {
        HTTP_DELETE = 0,
        HTTP_GET,
        HTTP_HEAD,
        HTTP_POST,
        HTTP_PUT
    };

    http_method get_method() const;
    const std::string get_url() const;
    const std::string get_http_version() const;
    std::map<std::string, std::string> get_headers() const;
    const std::string get_body() const;

    void connect_on_data(void (*function)(const char*, int));
    void connect_on_end(void (*function)());
private:
    boost::signals2::signal<void (const char *, int)> on_data;
    boost::signals2::signal<void ()> on_end;

    void set_finished(bool finished);
    void set_method(http_method method);
    void set_http_version(std::string http_version);
    void set_url(std::string url);
    void set_headers(std::map<std::string, std::string> headers);

    bool finished_;
    http_method method_;
    std::string http_version_;
    std::string url_;
    std::map<std::string, std::string> headers_;
};

#endif // HTTP_REQUEST_H
