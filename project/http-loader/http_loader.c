#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "loader_utils.h"
#include "tcp_loader_connection.h"
#include "http_client.h"


static
void write_to_file(struct RequestResult *req_result) {
  if (!req_result) {
      printf("Data for write is absent");
      return;
    }

  time_t cur_time = time(NULL);
  struct tm *local_time = localtime(&cur_time);
  char file_name[50] = "loaded_at_";
  strftime(file_name + strlen(file_name), 49 - strlen(file_name), "%s", local_time);
  printf("File name %s\n", file_name);

  FILE *out_file;
  out_file = fopen(file_name, "w");
  if (out_file) {
      fwrite(req_result->content, sizeof(char), req_result->size, out_file);
      fclose(out_file);
    } else {
      printf("Can't open a file %s\n", file_name);
    }
  return;
}


int main(int argc, char *argv[]) {
  if (argc != 2 ) {
      printf("Use: http_loader [http_url]\n");
      exit(EXIT_SUCCESS);
    }
  const char *url = argv[1];
  uint16_t port = 80;
  char *addr = NULL;
  char *path = NULL;

  if (parse_url(url, &addr, &port, &path) || !addr || !path) {
      printf("Bad parsing of url %s\n", url);
      free(addr);
      free(path);
      exit(EXIT_FAILURE);
    }

  struct hostent *host_entry;
  host_entry = gethostbyname(addr);
  if (!host_entry) {
      printf("Can't resolv address %s\n", addr);
      exit(EXIT_FAILURE);
    }

  char *ip_buff = NULL;
  if (host_entry->h_addrtype == AF_INET && host_entry->h_addr_list[0]) {
      ip_buff = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0]));
    }
  if (!ip_buff) {
      printf("Can't resolve address %s\n", addr);
      free(addr);
      free(path);
      exit(EXIT_FAILURE);
    }

  printf("Source %s:%d\n", ip_buff, port);

  struct LoaderConnection *conn = create_tcp_connection(ip_buff, port);

  if (!conn->loader_con_start(conn)) {
      printf("Can't connect to %s:%d\n", ip_buff, port);
      exit(EXIT_FAILURE);
    }

  struct HTTPClient *http_client = create_http_client(conn);
  if (!http_client) {
      printf("Can't create a http client\n");
      exit(EXIT_FAILURE);
    }

  struct RequestResult *result = http_client->load_resource(http_client, path, addr);
  if (!result) {
      printf("Can't load a resource\n");
      exit(EXIT_FAILURE);
    }

  write_to_file(result);
  free_request_result(result);
  result = NULL;

  destroy_http_client(http_client);
  http_client = NULL;
  conn->loader_con_stop(conn);
  destroy_tcp_connection(conn);
  conn = NULL;

  free(addr);
  free(path);
  exit(EXIT_SUCCESS);
}
