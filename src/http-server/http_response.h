#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H
#include "http_server_api.h"

#include "boost/signals2.hpp"

#include <string>
#include <map>

class http_response
{
    enum status_code
    {
            STATUS_CONTINUE = 100,
            STATUS_SWITCH_PROTOCOLS = 101,
            STATUS_OK = 200,
            STATUS_CREATED = 201,
            STATUS_ACCEPTED = 202,
            STATUS_NON_AUTHORITATIVE_INFORMATION = 203,
            STATUS_NO_CONTENT = 204,
            STATUS_RESET_CONTENT = 205,
            STATUS_PARTIAL_CONTENT = 206,
            STATUS_MULTIPLE_CHOICES = 300,
            STATUS_MOVED_PERMANENTLY = 301,
            STATUS_FOUND = 302,
            STATUS_SEE_OTHER = 303,
            STATUS_NOT_MODIFIED = 304,
            STATUS_USE_PROXY = 305,
            STATUS_TEMPORARY_REDIRECT = 307,
            STATUS_BAD_REQUEST = 400,
            STATUS_UNAUTHORIZED = 401,
            STATUS_PAYMENT_REQUIRED = 402,
            STATUS_FORBIDDEN = 403,
            STATUS_NOT_FOUND = 404,
            STATUS_METHOD_NOT_ALLOWED = 405,
            STATUS_NOT_ACCEPTABLE = 406,
            STATUS_PROXY_AUTHENTICATION_REQUIRED = 407,
            STATUS_REQUEST_TIMEOUT = 408,
            STATUS_CONFLICT = 409,
            STATUS_GONE = 410,
            STATUS_LENGTH_REQUIRED = 411,
            STATUS_PRECONDITION_FAILED = 412,
            STATUS_REQUEST_ENTITY_TOO_LARGE = 413,
            STATUS_REQUEST_URI_TOO_LONG = 414,
            STATUS_REQUEST_UNSUPPORTED_MEDIA_TYPE = 415,
            STATUS_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
            STATUS_EXPECTATION_FAILED = 417,
            STATUS_INTERNAL_SERVER_ERROR = 500,
            STATUS_NOT_IMPLEMENTED = 501,
            STATUS_BAD_GATEWAY = 502,
            STATUS_SERVICE_UNAVAILABLE = 503,
            STATUS_GATEWAY_TIMEOUT = 504,
            STATUS_HTTP_VERSION_NOT_SUPPORTED = 505
    };

    friend struct http_connection;
public:
    void set_header(const std::string &field, const std::string value);
    void write_head(status_code code);
    void write(const char * data);
    void done();
private:
    //TODO: add signals
    http_response(http_connection* connection);
    void write_header(std::string field, std::string value);
    void write_headers();

    http_connection* connection_;
    bool closed_;
    bool header_writen_;
    std::map<std::string, std::string> headers_;
};

#endif // HTTP_RESPONSE_H
