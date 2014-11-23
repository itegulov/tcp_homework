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

struct tcp_server {
public:
    bool begin_listening(char * address, char * service);
    void set_max_pending_connections(int max);
    tcp_socket* get_next_connection();
private:
    int max_pending_connections = 10;
    static int create_and_bind(char * address, char * service);
    static int make_socket_non_blocking(int socket_fd);
};
