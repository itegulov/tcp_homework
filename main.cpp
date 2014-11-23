#include <iostream>
#include "tcp_server.h"
using namespace std;

int main()
{
    tcp_server server;
    server.begin_listening("127.0.0.1", "3601");
    return 0;
}
