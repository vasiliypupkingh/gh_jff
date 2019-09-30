#ifndef HTTP_CLIENT
#define HTTP_CLIENT

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "loader_connection.h"

struct RequestResult {
  int result_code;
  char *content;
  size_t size;
};

void free_request_result(struct RequestResult *req_res);


struct HTTPClient {
  struct LoaderConnection *conn;

  struct RequestResult* (*load_resource)(struct HTTPClient*, const char*, const char*);
};

struct HTTPClient* create_http_client(struct LoaderConnection *conn);
void destroy_http_client(struct HTTPClient *http_client);

#endif
