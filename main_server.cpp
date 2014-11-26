#include "tcp_server.h"
#include "tcp_socket.h"

#include <csignal>

#include <signal.h>

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
                            "<h1>Nothing to see here</h1>";
    socket->write_data(response, strlen(response) + 1);
    socket->close();
}

tcp_server* server;

void sig_handler(int signum)
{
    delete server;
    exit(0);
}

int main() {
    struct sigaction new_action, old_action;
    new_action.sa_handler = sig_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    sigaction(SIGINT, nullptr, &old_action);
    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(SIGINT, &new_action, nullptr);
    }
    sigaction(SIGTERM, nullptr, &old_action);
    if (old_action.sa_handler != SIG_IGN)
    {
        sigaction(SIGTERM, &new_action, nullptr);
    }
    server = new tcp_server();
    server->new_connection.connect(&just_print);
    server->begin_listening("127.0.0.1", "20620");
    sleep(1000);
}
