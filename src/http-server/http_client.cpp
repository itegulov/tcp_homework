#include "http_client.h"
#include "tcp_socket.h"

http_client::http_client()
{
    client.on_connect.connect(
        [this](tcp_socket* socket)
        {
            socket->connect_on_read([this](tcp_socket* socket)
            {
                on_message(socket->read_all().c_str());
            });
        }

    );
}

void http_client::start(const char* url, const char* address, const char* service)
{
    client.on_connect.connect(
        [&](tcp_socket* socket)
        {
            on_connection();
            std::string s = "GET ";
            s += url;
            socket->write_all(s.c_str(), s.length());
        }
    );
    client.tcp_connect(address, service);
}

void http_client::write(const char *data, ssize_t size)
{
    client.write(data, size);
}
