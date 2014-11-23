#include "tcp_client.h"

void *tcp_client::get_in_addr(struct sockaddr *sa)
{
    return &(((struct sockaddr_in*)sa)->sin_addr);
}

bool tcp_client::tcp_connect(char * address, char * service)
{
    addrinfo hints, *servinfo, *p;
    const int MAX_DATA_SIZE = 100;
    char buf[MAX_DATA_SIZE];
    char s[INET_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int status = getaddrinfo(address, service, &hints, &servinfo);
    if (status != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        //TODO: check error
        return false;
    }
    int socket_fd;

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1)
        {
            perror("Client: socket");
            //TODO: check eror
            continue;
        }

        if (connect(socket_fd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(socket_fd);
            perror("Client: connect");
            //TODO: check eror
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Client: failed to connect\n");
        //TODO: check eror
        return false;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
                s, sizeof s);
    printf("Client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure
    int numbytes;
    if ((numbytes = recv(socket_fd, buf, MAX_DATA_SIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("Client: received '%s'\n", buf);

    close(socket_fd);

    return true;
}
