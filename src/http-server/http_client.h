#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "boost/signals2.hpp"
#include "tcp_client.h"
class http_client
{
public:
    http_client(const std::string& url, const std::string& address, const std::string& service, epoll_handler& handler);

    template<typename T>
    void connect_on_connection(T function)
    {
        on_connection.connect(function);
    }

    template<typename T>
    void connect_on_message(T function)
    {
        on_message.connect(function);
    }

    void write(const std::string& data);
private:
    boost::signals2::signal<void ()> on_connection;
    boost::signals2::signal<void (const char*)> on_message;

    tcp_client client;
};

#endif // HTTP_CLIENT_H
