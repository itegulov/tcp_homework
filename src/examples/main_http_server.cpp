#include "http_server.h"
#include "http_request.h"
#include "http_response.h"

#include <cstdio>
#include <iostream>
#include <csignal>

void on_request(http_request* request, http_response* response)
{
    std::cout << request->get_url() << " " << request->get_http_version() << std::endl;
    for (auto item : request->get_headers())
    {
        std::cout << item.first << ": " << item.second << std::endl;
    }
    std::cout << request->get_body() << std::endl;
    response->write_head(http_response::STATUS_OK);
    char data[100];
    sprintf(data, "<html><body>Sorry, there is no %s</body></html>", request->get_url().c_str());
    response->write(data, strlen(data));
    response->done();
}

epoll_handler handler;

void sig_handler(int signum)
{
    std::cout << "signal handler" << std::endl;
    handler.stop();
    std::cout << "stopped!" << std::endl;
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
    http_server server("127.0.0.1", "20621", &handler);
    server.connect_new_request(&on_request);
    handler.start();
}
