#include "tcp_server.h"
#include "tcp_socket.h"

#include <csignal>
#include <iostream>

#include <signal.h>

void just_get(tcp_socket *socket);

void just_print(tcp_socket* socket)
{
    printf("accepted_fd: %d\n", socket->get_descriptor());
    fflush(stdout);
    socket->on_read.connect(&just_get);
}

void just_get(tcp_socket* socket)
{
    const char* s = socket->read_all();
    if (socket->is_open())
    {
        std::cout << s << std::endl;
        socket->write_all(s, strlen(s));
    }
    delete[] s;
}

void on_error(const std::exception& e)
{
    printf("%s\n", e.what());
    fflush(stdout);
}

tcp_server* server;
epoll_handler* handler;

void sig_handler(int signum)
{
    handler->stop();
}

int main()
{
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

    handler = new epoll_handler();
    server = new tcp_server();
    server->new_connection.connect(&just_print);
    server->on_error.connect(&on_error);
    server->start("127.0.0.1", "20620", handler);
    handler->start();
    delete handler;
    delete server;
}
