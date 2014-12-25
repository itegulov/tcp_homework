#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H
#include "boost/signals2.hpp"
#include "tcp_client.h"
class http_client
{
public:
    http_client();
    void start(const char * url, const char * address, const char * service);

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

    void write(const char * data, ssize_t size);
private:
    boost::signals2::signal<void ()> on_connection;
    boost::signals2::signal<void (const char*)> on_message;

    tcp_client client;
};

#endif // HTTP_CLIENT_H
