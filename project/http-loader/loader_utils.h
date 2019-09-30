#ifndef LOADER_UTILS
#define LOADER_UTILS

#include <stdint.h>


int parse_url(const char *url, char **addr, uint16_t *port, char **path);

#endif
