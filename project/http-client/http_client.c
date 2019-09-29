#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "http_client.h"


struct HeaderParseResult{
  int result_code;
  size_t content_length;
};


const size_t request_buff_size = 1024;
const size_t response_header_buff_size = 10 * 1024;
const size_t max_content_size = 5 * 1024 * 1024;

static
bool http_request_str(const char *path, const char *host, char *buff, size_t buff_size) {
  if (!path || !host) {
      printf("Can't load a resource without path or host\n");
      return false;
    }

  if (strlen(path) > 200 || strlen(host) > 200) {
      printf("Host %s or path %s greater 200 charecters\n", host, path);
      return false;
    }

  if (buff_size < request_buff_size) {
      printf("In http_request_str. Buffer is too small\n");
      return false;
    }
  buff[0] = 0;
  strcat(buff, "GET ");
  strcat(buff, path);
  strcat(buff, " HTTP/1.1\r\n");
  strcat(buff, "User-Agent: dummy http loader\r\n"
               "Accept: text/html\r\n"
               "Accept-Encoding: 0\r\n"
               "Connection: Keep-Alive\r\n"
               "Host: ");
  strcat(buff, host);
  strcat(buff, "\r\n\r\n");

  return true;
}


static
int parse_http_headers(const char* buff, struct HeaderParseResult *parse_result, const char** end_of_header) {
  if (!buff || !parse_result || !end_of_header) {
      printf("Invalid arguments in parse_http_headers\n");
      return -1;
    }

  const char *end = strstr(buff, "\r\n\r\n");
  if (!end) {
      return 0;
    }

  *end_of_header = end + strlen("\r\n\r\n");
  const char *code_str = strstr(buff, "HTTP/1.1 ");

  if (!code_str) {
      printf("Can't find a code result\n");
      return -1;
    }

  if (end <= code_str + strlen("HTTP/1.1 ")) {
      return -1;
    }
  code_str += strlen("HTTP/1.1 ");

  unsigned long code_result = strtoul(code_str, NULL, 10);
  if (!code_result) {
      return -1;
    }

  parse_result->result_code = (int)code_result;

  const char *content_len_str = strstr(buff, "Content-Length: ");
  if (!content_len_str) {
      printf("Can't find a Content-Length header\n");
      return -1;
    }
  if (end <= content_len_str + strlen("Content-Length: ")) {
      return -1;
    }
  content_len_str += strlen("Content-Length: ");

  unsigned long content_len = strtoul(content_len_str, NULL, 10);
  if (!content_len) {
      return -1;
    }

  parse_result->content_length = content_len;
  return 1;
}


static
struct RequestResult* load_resource(struct HTTPClient* http_client, const char *path, const char *host) {
  if (!http_client) {
      printf("HTTP client is absent\n");
      return NULL;
    }

  char http_request[request_buff_size];
  if (!http_request_str(path, host, http_request, request_buff_size)) {
      printf("Can't create http request\n");
      return NULL;
  }

  printf("Request\n%s\n", http_request);

  http_client->conn->loader_send(http_client->conn, http_request, strlen(http_request));


  char header_buff[response_header_buff_size];
  struct HeaderParseResult parse_result;
  memset(&parse_result, 0, sizeof(struct HeaderParseResult));
  memset(header_buff, 0, response_header_buff_size);
  ssize_t cur_recv = 0;
  size_t sum_recv = 0;
  bool timeout = false;
  int parse_done = 0;
  const char *end_of_headers;

  do {
      cur_recv = http_client->conn->loader_recv(http_client->conn, header_buff + sum_recv,
                                                (response_header_buff_size - 1) - sum_recv, false, &timeout);
      sum_recv += cur_recv;
      if (timeout) {
          printf("Timeout was reached\n");
          return NULL;
        }
      if (cur_recv == -1 || cur_recv == 0) {
          printf("Some error occured\n");
          return NULL;
        }
      parse_done = parse_http_headers(header_buff, &parse_result, &end_of_headers);

      if ((!parse_done && sum_recv == response_header_buff_size) || parse_done == -1) {
          printf("Can't parse http header\n");
          return NULL;
        }

    } while (!parse_done);

    if (!parse_result.result_code || !parse_result.content_length) {
        printf("Parsing of HTTP headers failed\n");
        return NULL;
      }

    if (parse_result.content_length > max_content_size) {
        printf("Content size is so big %lu\n", parse_result.content_length);
        return NULL;
      }


    struct RequestResult *result = malloc(sizeof(struct RequestResult));
    memset(result, 0, sizeof(struct RequestResult));

    result->result_code = parse_result.result_code;
    result->size = parse_result.content_length;
    result->content = malloc(parse_result.content_length);

    const char *header_buff_end = header_buff + sum_recv;
    size_t rcv_content_bytes = 0;

    if (header_buff_end - end_of_headers > 0) {
        rcv_content_bytes = header_buff_end - end_of_headers;
        strncpy(result->content, end_of_headers, header_buff_end - end_of_headers);
      }

    if (rcv_content_bytes < result->size) {
        timeout = false;
        ssize_t content_recv = http_client->conn->loader_recv(http_client->conn, result->content + rcv_content_bytes,
                                                              result->size - rcv_content_bytes, true, &timeout);

        if (timeout || content_recv == -1) {
            free_request_result(result);
            return NULL;
          }
      }

    return result;
}


struct HTTPClient* create_http_client(struct LoaderConnection *conn) {
  if (!conn) {
      return NULL;
    }

  struct HTTPClient *http_client = malloc(sizeof(struct HTTPClient));
  http_client->conn = conn;
  http_client->load_resource = load_resource;

  return http_client;
}


void destroy_http_client(struct HTTPClient *http_client) {
  free(http_client);
  return;
}


void free_request_result(struct RequestResult *req_res) {
  if (!req_res) {
      return;
    }

  if (req_res->content) {
      free(req_res->content);
    }

  return;
}
