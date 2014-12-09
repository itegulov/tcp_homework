#include "tcp_client.h"

void *tcp_client::get_in_addr(sockaddr *sa)
{
    return &(((sockaddr_in*)sa)->sin_addr);
}

void tcp_client::tcp_connect(const char * address, const char * service)
{
    addrinfo hints, *servinfo, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address, service, &hints, &servinfo);
    if (status != 0)
    {
        throw tcp_exception(gai_strerror(status));
    }
    int socket_fd;

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1)
        {
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        throw tcp_exception("Failed to connect");
    }
    tcp_socket socket(socket_fd, nullptr);

    freeaddrinfo(servinfo);
    on_connect(&socket);
}