#include "http_server.h"

#include <csignal>
#include <cstdio>
#include <iostream>

void on_request(http_request& request, http_response& response)
{
    std::cout << "New request:" << std::endl;
    std::cout << request.get_method() << std::endl;
    std::cout << request.get_url() << std::endl;
    std::cout << request.get_major_version() << " " << request.get_minor_version() << std::endl;
    const std::map<std::string, std::string>& headers = request.get_headers();
    for (auto header : headers)
    {
        std::cout << header.first << ": " << header.second << std::endl;
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
    http_server server("127.0.0.1", "20623", handler);
    //server.connect_on_body(&on_body);
    //server.connect_on_head(&on_head);
    //server.connect_new_request(&on_request);
    server.connect_on_request(&on_request);
    handler.start();
}
