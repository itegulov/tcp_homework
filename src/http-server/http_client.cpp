#include "http_client.h"
#include "tcp_socket.h"

http_client::http_client(const std::string& url, const std::string& address, const std::string& service, epoll_handler& handler): client(address, service, handler)
{
    client.connect_on_connect(
        [this](tcp_socket* socket)
        {
            socket->connect_on_read([this](tcp_socket* socket)
            {
                on_message(socket->read_all().c_str());
            });
        }

    );
    client.connect_on_connect(
        [&](tcp_socket* socket)
        {
            on_connection();
            std::string s = "GET ";
            s += url;
            s += " HTTP/1.1\n";
            socket->write_all(s);
        }
    );
    client.connect_on_message(
                [&](tcp_socket* socket)
    {
        on_message(socket->read_all().c_str());
    });
}

void http_client::write(const std::string& data)
{
    client.write(data);
}
