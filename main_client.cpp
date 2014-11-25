#include "tcp_client.h"

int main() {
    for (int i = 0; i < 5; i++)
    {
        fork();
    }
    tcp_client client;
    client.tcp_connect("127.0.0.1", "20628");
}
