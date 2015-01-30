#ifndef TCP_SERVER_API_H
#define TCP_SERVER_API_H
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

#include <type_traits>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <memory>
#include <functional>
#include <map>
#include <vector>
struct tcp_server;
struct tcp_client;
struct tcp_socket;
struct tcp_exception;
struct epoll_handler;
#endif // TCP_SERVER_API_H
