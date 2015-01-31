#include "http_server.h"
#include "http_request.h"
#include "http_response.h"

#include <cstdio>
#include <iostream>
#include <csignal>

epoll_handler handler;
http_server server("127.0.0.1", "24500", handler);

void on_request(http_request& request, http_response& response)
{
    response.write_head(http_response::STATUS_OK);
}

void on_body(http_request& request, const std::string& data, http_response& response)
{
    std::vector<std::pair<http_request&, http_response&> > vector = server.get_connections();
    const std::map<std::string, std::string>& headers = (request).get_headers();
    std::string s = (*headers.find("Host")).second;
    std::string new_s = s + ": " + data;
    for (auto it = vector.begin(); it != vector.end(); it++)
    {
        (*it).second.write(new_s);
    }
}

void on_error(const std::exception& e) {

}

void sig_handler(int signum)
{
    handler.stop();
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
    server.connect_on_error(&on_error);
    server.connect_on_request(&on_request);
    server.connect_on_body(&on_body);
    handler.start();
}
