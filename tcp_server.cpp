#include "tcp_server.h"

bool tcp_server::begin_listening(char *address, char * service)
{
    int socket_fd = create_and_bind(address, service);
    if (socket_fd == -1)
    {
        //TODO: check error
        return false;
    }
    int status = listen(socket_fd, max_pending_connections);

    if (status == -1)
    {
        //TODO: check error
        close(socket_fd);
        return false;
    }
    while (true)
    {
        sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        int accepted_fd = accept(socket_fd, (struct sockaddr *)&their_addr, &addr_size);
        if (accepted_fd == -1)
        {
            //TODO: check error
            continue;
        }
        char s[INET_ADDRSTRLEN];
        inet_ntop(their_addr.ss_family,
                    &(((struct sockaddr_in*)&their_addr)->sin_addr),
                    s, sizeof s);
        printf("Server: got connection from %s\n", s);
        if (!fork()) { // this is the child process
            close(socket_fd); // child doesn't need the listener
            if (send(accepted_fd, "Hello, world!", 13, 0) == -1)
                perror("send");
            close(accepted_fd);
            return true;
        }
        close(accepted_fd);
    }
    close(socket_fd);
    return true;
}

int tcp_server::create_and_bind(char * address, char * service)
{
    int status = 0;
    addrinfo hints;
    addrinfo * servinfo;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(address, service, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        //TODO: check error
        return -1;
    }

    struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(servinfo->ai_family, &(ipv4->sin_addr), ipstr, sizeof ipstr);
    printf("Server: starting on ip %s and port %s...\n", ipstr, service);
    int socket_fd = -1;
    addrinfo* rp;
    for (rp = servinfo; rp != nullptr; rp = rp->ai_next)
    {
        socket_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (socket_fd == -1)
        {
            continue;
        }

        status = bind(socket_fd, servinfo->ai_addr, servinfo->ai_addrlen);

        if (status == 0)
        {
           break;
        }

        close(socket_fd);
    }
    if (rp == nullptr)
    {
        //TODO: check error
        return -1;
    }
    freeaddrinfo(servinfo);
    return socket_fd;
}

int tcp_server::make_socket_non_blocking(int socket_fd)
{
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1)
    {
        //TODO: check error
        return -1;
    }
    flags |= O_NONBLOCK;
    int status = fcntl(socket_fd, F_SETFL, flags);
    if (status == -1)
    {
        //TODO: check error
        return -1;
    }
    return 0;
}

void tcp_server::set_max_pending_connections(int max)
{
    max_pending_connections = max;
}
