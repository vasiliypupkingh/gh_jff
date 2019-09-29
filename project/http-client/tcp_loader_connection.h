#ifndef TCP_LOADER_CONNECTION
#define TCP_LOADER_CONNECTION

#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>

#include "loader_connection.h"


struct TcpLoaderConnection {
  struct LoaderConnection base_conn;

   int sockfd;
   struct sockaddr_in serv_addr;
};

struct LoaderConnection* create_tcp_connection(const char* addr, uint16_t port);
void destroy_tcp_connection(struct LoaderConnection* connection);

#endif
