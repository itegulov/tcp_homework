#include "tcp_client.h"

void on_connect(tcp_socket* socket)
{
    const char * buf = "Hello, world!\n";
    socket->write_data(buf, 14);
    char new_buf[512];
    socket->read_data(new_buf, 512);
    printf("Client: received '%s'\n", new_buf);
    const char * buf2 = "Let's do it!\n";
    socket->write_data(buf2, 14);
    char new_buf2[512];
    socket->read_data(new_buf2, 512);
    printf("Client: received '%s'\n", new_buf2);
    fflush(stdout);
}

int main()
{
    tcp_client client;
    client.on_connect.connect(on_connect);
    client.tcp_connect("127.0.0.1", "20619");
}
