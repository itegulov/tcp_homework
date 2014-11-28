#include "tcp_client.h"

void on_connect(tcp_socket* socket)
{
    const char * buf1 = "Hello, world!\0";
    socket->write_data(buf1, strlen(buf1));
    char new_buf1[512];
    socket->read_data(new_buf1, 512);
    printf("Client: received '%s'\n", new_buf1);
    fflush(stdout);
    const char * buf2 = "Good bye, world!\0";
    socket->write_data(buf2, strlen(buf2));
    char new_buf2[512];
    socket->read_data(new_buf2, 512);
    printf("Client: received '%s'\n", new_buf2);
    fflush(stdout);
}

int main()
{
    tcp_client client;
    client.on_connect.connect(on_connect);
    client.tcp_connect("127.0.0.1", "20620");
}
