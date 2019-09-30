#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "loader_utils.h"


int parse_url(const char *url, char **addr, uint16_t *port, char **path) {
  if (!url || !addr || !port || !path) {
      printf("Invalid arguments in parse_url");
      return 1;
    }

  const char valid_scheme[] = "http://";

  if (strlen(url) <= strlen(valid_scheme) || strstr(url, valid_scheme) != url) {
      printf("Can't parse url %s\n", url);
      return 1;
    }

  const char *addr_start = url + strlen(valid_scheme);
  const char *addr_finish = NULL;

  const char *colon_ptr = index(addr_start, ':');

  if (colon_ptr) {
      const char *port_start = colon_ptr + 1;
      addr_finish = colon_ptr;
      char **endptr = NULL;
      unsigned long port_tmp = strtoul(port_start, endptr, 10);

      if (addr_start == addr_finish || port_tmp > UINT16_MAX || (endptr && *endptr == port_start)) {
          printf("Can't parse url %s\n", url);
          return 1;
        }
      *port = (uint16_t)port_tmp;
    }

  const char *path_start = index(addr_start, '/');
  const char *path_finish = url + strlen(url);

  if (path_start) {
      if (!addr_finish) {
          addr_finish = path_start;
        }
      const char *arg_start = index(path_start, '?');
      if (arg_start) {
          path_finish = arg_start;
        }
    }

  if (!path_start || path_start == path_finish) {
      *path = strdup("/");
    } else {
      *path = strndup(path_start, path_finish - path_start);
    }

  if (!addr_finish) {
      const char *arg_start = index(addr_start, '?');
      if (arg_start) {
          addr_finish = arg_start;
        } else {
          addr_finish = url + strlen(url);
        }
    }

  if (addr_start == addr_finish) {
      printf("Can't parse url %s\n", url);
      return 1;
    }

  *addr = strndup(addr_start, addr_finish - addr_start);

  return 0;
}
