#ifndef LOADER_CONNECTION
#define LOADER_CONNECTION

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>


struct LoaderConnection {
  bool (*loader_start)(struct LoaderConnection*);
  void (*loader_stop)(struct LoaderConnection*);
  ssize_t(*loader_send)(struct LoaderConnection*, char*, size_t);
  ssize_t(*loader_recv)(struct LoaderConnection*, char*, size_t, bool, bool*);
};

#endif
