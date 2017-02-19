#pragma once

#include <netinet/in.h>

#include <map>
#include <mutex>
#include <memory>
#include <vector>

namespace tcp_srv {

  class tcp_srv {
  private:
    static const int _max_events = 32;

    int _MasterSocket;
    sockaddr_in _srv_addr;
    int _epoll_ds;
    std::map<int, sockaddr> _clients;
    std::mutex _clients_mut;

    void (*_handler)(int fd, tcp_srv *serv, void *args);
    void *_handler_args;

    static int _set_nonblock(int fd);
    static void _add_connection(tcp_srv *serv);
    static void _handle_connection(int fd, tcp_srv *serv);

  public:
    tcp_srv() = delete;
    tcp_srv(tcp_srv&&) = delete;
    tcp_srv(tcp_srv&) = delete;

    tcp_srv(std::string ipv4_str, uint16_t port);
    void start();
    int tcp_recv(int fd, std::shared_ptr<std::vector<uint8_t> > message);
    int tcp_send(int fd, std::shared_ptr<std::vector<uint8_t> > message);
    void delete_connection(int fd);
    void set_handler(void (*handler)(int, tcp_srv *, void *), void *args)
    {
      _handler_args = args;
      _handler = handler;
    }
  };

}
