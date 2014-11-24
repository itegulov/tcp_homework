#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "cstdio"
#include "cstring"
#include "cstdlib"

struct tcp_client {
public:
    bool tcp_connect(char * address, char * service);
private:
    static void *get_in_addr(struct sockaddr *sa);
};
#endif //TCP_CLIENT_H
