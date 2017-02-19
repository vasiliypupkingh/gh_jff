#include "tcp_srv.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <vector>
#include <iostream>
#include <thread>
#include <string>
#include <stdexcept>

#include <boost/log/trivial.hpp>

namespace tcp_srv {

  tcp_srv::tcp_srv(std::string ipv4_str, uint16_t port)
    :_handler(nullptr)
  {
    _MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    _srv_addr.sin_family = AF_INET;
    _srv_addr.sin_port = ntohs(port);
    inet_pton(AF_INET, ipv4_str.c_str(), &(_srv_addr.sin_addr));

    bind(_MasterSocket, (sockaddr *) &_srv_addr, sizeof(_srv_addr));
    _set_nonblock(_MasterSocket);

    _epoll_ds = epoll_create1(0);
  }


  void
  tcp_srv::start()
  {
    listen(_MasterSocket, SOMAXCONN);
    epoll_event ep_event;
    ep_event.data.fd = _MasterSocket;
    ep_event.events = EPOLLIN;
    epoll_ctl(_epoll_ds, EPOLL_CTL_ADD, _MasterSocket, &ep_event);

    while(true)
      {
        epoll_event ep_events[_max_events];
        int N = epoll_wait(_epoll_ds, ep_events, _max_events, -1);

        std::vector<std::thread> vec_of_threads;

        for(int i = 0; i < N; ++i)
          {
            if(ep_events[i].data.fd == _MasterSocket)
              {
                vec_of_threads.emplace_back(std::thread(_add_connection, this));
              }
            else
              {
                vec_of_threads.emplace_back(std::thread(_handle_connection,
                                                        static_cast<int>(ep_events[i].data.fd),
                                                        this));
              }
          }

        for(auto& th : vec_of_threads)
          th.join();
      }
  }


  void
  tcp_srv::_handle_connection(int fd, tcp_srv *serv)
  {
    if(serv->_handler != nullptr)
      {
        serv->_handler(fd, serv, serv->_handler_args);
      }
    else
      {
        throw std::runtime_error("Handle for tcp connection is not set");
      }
  }


  void
  tcp_srv::_add_connection(tcp_srv *serv)
  {
    sockaddr client_addr;
    socklen_t len = sizeof(client_addr);

    int SlaveSocket = accept(serv->_MasterSocket, (sockaddr *) &client_addr, &len);

    _set_nonblock(SlaveSocket);
    epoll_event ep_ev_slave;
    ep_ev_slave.data.fd = SlaveSocket;
    ep_ev_slave.events = EPOLLIN;
    epoll_ctl(serv->_epoll_ds, EPOLL_CTL_ADD, SlaveSocket, &ep_ev_slave);

    std::unique_lock<std::mutex> lock(serv->_clients_mut);
    serv->_clients.emplace(SlaveSocket, client_addr);
  }


  void
  tcp_srv::delete_connection(int fd)
  {
    shutdown(fd, SHUT_RDWR);
    close(fd);

    std::unique_lock<std::mutex> lock(_clients_mut);
    _clients.erase(fd);
  }


  int
  tcp_srv::tcp_recv(int fd, std::shared_ptr<std::vector<uint8_t> > message)
  {
    uint8_t Buf[1500];
    ssize_t recv_bytes;

    do
      {
        recv_bytes = recv(fd, Buf, 1500, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (recv_bytes > 0)
          message->insert(message->end(), Buf, Buf + recv_bytes);
      } while(recv_bytes > 0);

    return 0;  //it can return some code of a error or OK = 0
  }


  int
  tcp_srv::tcp_send(int fd, std::shared_ptr<std::vector<uint8_t> > message)
  {
    ssize_t send_bytes = 0;
    size_t sum_of_send = 0;

    do
      {
        send_bytes = send(fd, message->data() + send_bytes,
                          message->size() - send_bytes, MSG_NOSIGNAL);
        if (send_bytes > 0)
          sum_of_send += send_bytes;
      } while(sum_of_send != message->size());

    return 0;  //it can return some code of a error or OK = 0
  }


  int
  tcp_srv::_set_nonblock(int fd)
  {
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
      flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1
        return ioctl(fd, FIOBIO, &flags);
#endif
  }

}
