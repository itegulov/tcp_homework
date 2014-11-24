#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>


#include <boost/signals2.hpp>

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <exception>

struct tcp_server
{
public:
    ~tcp_server();
    bool begin_listening(char * address, char * service);
    void stop_listening();
    void set_max_pending_connections(int max);
    boost::signals2::signal<void (int)> new_connection;
    boost::signals2::signal<void (int)> close_connection;
    boost::signals2::signal<void (int, char*, int)> on_read;
private:
    const int MAX_EVENTS = 64;
    int max_pending_connections = 20;
    int epoll_fd;
    bool running = false;
    std::thread* t;
    void run(int socket_fd, epoll_event* events);
    static int create_and_bind(char * address, char * service);
    static int make_socket_non_blocking(int socket_fd);
};

struct tcp_exception: public std::exception
{
public:
    tcp_exception(const char * description);
    virtual const char* what() const throw();
private:
    const char * description;
};
#endif //TCP_SERVER_H

