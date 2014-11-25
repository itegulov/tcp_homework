#include "tcp_server.h"
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <stdint.h>

#include "tcp_socket.h"

void just_get(tcp_socket *socket);

void just_print(tcp_socket* socket)
{
    printf("accepted_fd: %d\n", socket->get_socket_descriptor());
    fflush(stdout);
    socket->on_read.connect(&just_get);
}

void just_get(tcp_socket* socket)
{
    std::string s = socket->read_all();
    ssize_t count = write(1, s.c_str(), strlen(s.c_str()));
    const char * response = "HTTP/1.0 200 Ok\r\n"
                            "Content-Type: text/html; charset=\"utf-8\"\r\n"
                            "\r\n"
                            "<h1>Nothing to see here</h1>\n";
    socket->write_all(response, strlen(response));
    socket->close_socket();
}

tcp_server* server;

void sig_handler(int x)
{
    delete server;
    exit(0);
}

int main() {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGTSTP, sig_handler);
    server = new tcp_server();
    server->new_connection.connect(&just_print);
    server->begin_listening("127.0.0.1", "20629");
    sleep(1000);
}
