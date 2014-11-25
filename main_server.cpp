#include "tcp_server.h"
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <stdint.h>

#include "tcp_socket.h"

void just_get(tcp_socket *socket);

void just_print(tcp_socket* socket)
{
    printf("accepted_fd: %d\n", socket->get_socket_descriptor());
    fflush(stdout);
    socket->on_read.connect(&just_get);
}

void just_get(tcp_socket* socket)
{
    bool need_close = false;
    while (true)
    {
        ssize_t count;
        char buf[512];

        count = read (socket->get_socket_descriptor(), buf, sizeof buf);
        if (count == -1)
        {
            if (errno != EAGAIN)
            {
                need_close = true;
            }
            break;
        }
        else if (count == 0)
        {
            //Reached end of stream (closed connection)
            need_close = true;
            break;
        }

        int status = write(1, buf, count);
        if (status == -1)
        {
            perror ("write");
            return;
        }

        status = write(socket->get_socket_descriptor(), buf, count);
        if (status == -1)
        {
            perror ("write");
            return;
        }
    }

    if (need_close)
    {
        printf ("Closed connection on descriptor %d\n",
                socket->get_socket_descriptor());
        //Closing fd -> removing it from epoll fd
        close (socket->get_socket_descriptor());
    }
}

tcp_server* server;

void sig_handler(int x)
{
    delete server;
    exit(0);
}

int main() {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGTSTP, sig_handler);
    server = new tcp_server();
    server->new_connection.connect(&just_print);
    server->begin_listening("127.0.0.1", "20628");
    sleep(1000);
}
