#ifndef LOADER_CONNECTION
#define LOADER_CONNECTION

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>


struct LoaderConnection {
  bool (*loader_con_start)(struct LoaderConnection*);
  void (*loader_con_stop)(struct LoaderConnection*);
  ssize_t(*loader_con_send)(struct LoaderConnection*, char*, size_t);
  ssize_t(*loader_con_recv)(struct LoaderConnection*, char*, size_t, bool, bool*);
};

#endif
