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
    std::cout << "New request:" << std::endl;
    std::cout << request.get_method() << std::endl;
    std::cout << request.get_url() << std::endl;
    std::cout << request.get_major_version() << " " << request.get_minor_version() << std::endl;
    const std::map<std::string, std::string>& headers = request.get_headers();
    bool alive = true;
    for (auto header : headers)
    {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    response.write_head(http_response::STATUS_OK);
    if (!alive)
    {
        response.done();
    }
}

void on_body(http_request& request, const std::string& data, http_response& response)
{
    std::cout << "Going to response" << std::endl;
    std::vector<std::pair<http_request&, http_response&> > vector = server.get_connections();
    const std::map<std::string, std::string>& headers = (request).get_headers();
    std::string s = (*headers.find("Host")).second;
    std::string new_s = s + ": " + data;
    for (auto it = vector.begin(); it != vector.end(); it++)
    {
        std::cout << (*it).first.get_url() << std::endl;
        (*it).second.write(new_s);
    }
}

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
    server.connect_on_request(&on_request);
    server.connect_on_body(&on_body);
    handler.start();
}
