#include "tcp_server.h"

void just_print(int accepted_fd)
{
    printf("accepted_fd: %d\n", accepted_fd);
    fflush(stdout);
}

void just_response(int accepted_fd, char * buf, int count)
{
    int status = write(1, buf, count);
    if (status == -1)
    {
        perror ("write");
        return;
    }

    status = write(accepted_fd, buf, count);
    if (status == -1)
    {
        perror ("write");
        return;
    }
}

int main() {
    tcp_server server;
    server.new_connection.connect(&just_print);
    server.on_read.connect(&just_response);
    server.begin_listening("127.0.0.1", "20628");
    sleep(10);
    server.stop_listening();
}
