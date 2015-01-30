#include "tcp_client.h"
#include <csignal>
#include <iostream>

void on_connect(tcp_socket& socket)
{
    socket.write_all("Hello, world!\0");
}

void on_message(tcp_socket& socket)
{
    std::string s = socket.read_all();
    std::cout << "Client: received '" << s << "'" << std::endl;
    if (s == "Hello, world!")
    {
        socket.write_all("Good bye, world!\0");
    }
    else
    {
        socket.close();
    }
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
    tcp_client client("127.0.0.1", "20621", handler);
    client.connect_on_connect(&on_connect);
    client.connect_on_message(&on_message);
    client.connect();
    handler.start();
}
