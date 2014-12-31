#include "tcp_server.h"
#include "tcp_socket.h"

#include <csignal>
#include <iostream>

#include <signal.h>

void just_get(tcp_socket& socket)
{
    std::string s = socket.read_all();
    if (socket.is_open())
    {
        std::cout << s << std::endl;
        socket.write_all(s);
    }
}
void just_print(tcp_socket& socket)
{
    printf("accepted_fd: %d\n", socket.get_descriptor());
    fflush(stdout);
    socket.connect_on_read(&just_get);
}

void on_error(const std::exception& e)
{
    printf("%s\n", e.what());
    fflush(stdout);
}

epoll_handler handler;

void sig_handler(int signum)
{
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
    tcp_server server("127.0.0.1", "20621", handler, 20);
    server.connect_on_connection(&just_print);
    server.connect_on_error(&on_error);
    handler.start();
}
