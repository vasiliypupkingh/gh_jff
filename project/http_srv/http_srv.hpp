#pragma once

#include <unistd.h>

#include <string>
#include <vector>

#include "tcp_srv.hpp"

namespace http_srv {

  class http_srv {
  private:
    tcp_srv::tcp_srv _tcp_serv;

    static std::vector<std::string> _parse_http_req(const std::string& request);
    static std::vector<std::string> _parse_http_start_str(const std::string& request);
    static std::string _handle_http_get(const std::vector<std::string> &request);
    static bool _validate_uri(std::string &path);
    static std::string _response_200(const std::string &proto, std::string path);
    static std::string _response_404(const std::string &proto)
    {
      return  std::string(proto + " 404 Not Found\r\n\r\n");
    }
    static std::string _response_400()
    {
      return  std::string("HTTP/1.1 400 Bad Request\r\n\r\n");
    }

    static void _handler(int fd, tcp_srv::tcp_srv *serv, void *args);

  public:
    http_srv() = delete;
    http_srv(http_srv&&) = delete;
    http_srv(http_srv&) = delete;

    http_srv(std::string ipv4_str, uint16_t port, std::string root)
      :_tcp_serv(ipv4_str.c_str(), port)
    {
      if (!root.empty())
        {
          if (chdir(root.c_str()) != 0)
            throw;
        }
    }

    void start();
  };

}
