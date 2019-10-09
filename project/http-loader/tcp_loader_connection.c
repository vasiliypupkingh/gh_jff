#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "tcp_loader_connection.h"

static const unsigned int read_timeout_sec = 3;
static const unsigned int connect_timeout_sec = 5;


static
void connection_fail(int signum) {
  (void)signum;
  printf("Can't connect to server\n");
  exit(EXIT_FAILURE);
}


static
bool tcp_loader_con_start(struct LoaderConnection* connection){
  if (!connection) {
      printf("Invalid pointer in start connection\n");
      return false;
    }

  struct TcpLoaderConnection *conn = (struct TcpLoaderConnection*)connection;
  conn->sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (conn->sockfd == -1) {
      printf("Socket creation failed\n");
      return false;
    }

  signal(SIGALRM, connection_fail);
  alarm(connect_timeout_sec);
  if (connect(conn->sockfd, (const struct sockaddr *)(&conn->serv_addr), sizeof(conn->serv_addr))) {
      alarm(0);
      printf("Can't connect with server\n");
      return false;
    }
    alarm(0);
    printf("Connected with server\n");

  return true;
}


static
void tcp_loader_con_stop(struct LoaderConnection* connection) {
  if (!connection) {
      printf("Invalid pointer in stop connection\n");
      return;
    }
  struct TcpLoaderConnection *conn = (struct TcpLoaderConnection*)connection;
  close(conn->sockfd);
  printf("Connection stoped\n");
  return;
}


static
ssize_t tcp_loader_con_send(struct LoaderConnection* connection, char* buff, size_t buff_size) {
  if (!connection) {
      printf("Invalid pointer in loader send\n");
      return -1;
    }
  struct TcpLoaderConnection *conn = (struct TcpLoaderConnection*)connection;
  ssize_t send_bytes = 0;
  size_t sum_of_send = 0;

  do
    {
      send_bytes = send(conn->sockfd, buff + send_bytes,
                        buff_size - send_bytes, 0);
      if (send_bytes > 0)
        sum_of_send += send_bytes;
    } while(sum_of_send != buff_size);

  return sum_of_send;
}


static
ssize_t tcp_loader_con_recv(struct LoaderConnection* connection, char* buff, size_t buff_size, bool try_all, bool *timeout) {
  if (!connection) {
      printf("Invalid pointer in loader recv\n");
      return -1;
    }
  struct TcpLoaderConnection *conn = (struct TcpLoaderConnection*)connection;
  if (timeout) {
      *timeout = false;
    }

  fd_set rfds;
  struct timeval tv;
  tv.tv_sec = read_timeout_sec;
  tv.tv_usec = 0;

  ssize_t readed = 0;
  size_t sum_readed = 0;

  do {
      FD_ZERO(&rfds);
      FD_SET(conn->sockfd, &rfds);

      int result = select(conn->sockfd + 1, &rfds, NULL, NULL, &tv);
      if (!result) {
          printf("Timeout expires\n");
          if (timeout) {
              *timeout = true;
            }
          break;
        }
      if (result == -1) {
          perror("TCP recv");
          readed = -1;
          break;
        }
      readed = recv(conn->sockfd, buff + sum_readed, buff_size - sum_readed, MSG_DONTWAIT);
      sum_readed += readed;
    } while (try_all && sum_readed != buff_size);

  if (readed == -1 || readed == 0) {
      printf("Something wrong in tcp recv\n");
      return readed;
    }

  return sum_readed;
}


struct LoaderConnection* create_tcp_connection(const char *addr, uint16_t port) {
  struct TcpLoaderConnection *conn = malloc(sizeof(struct TcpLoaderConnection));

  if (!addr) {
      printf("Server address is absent\n");
      return NULL;
    }

  conn->base_conn.loader_con_start = tcp_loader_con_start;
  conn->base_conn.loader_con_stop = tcp_loader_con_stop;
  conn->base_conn.loader_con_send = tcp_loader_con_send;
  conn->base_conn.loader_con_recv = tcp_loader_con_recv;

  bzero(&conn->serv_addr, sizeof(conn->serv_addr));
  conn->serv_addr.sin_family = AF_INET;
  conn->serv_addr.sin_addr.s_addr = inet_addr(addr);
  conn->serv_addr.sin_port = htons(port);

  return (struct LoaderConnection*)conn;
}


void destroy_tcp_connection(struct LoaderConnection *connection) {
  free(connection);
}
