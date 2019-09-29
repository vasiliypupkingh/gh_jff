C=gcc
CFLAGS=-Wall -Wextra -Werror

all: http_loader

http_loader:
	$(C) $(CFLAGS) -o http_loader http_loader.c loader_utils.c tcp_loader_connection.c http_client.c

.PHONY: clean

clean:
	rm -f http_loader
