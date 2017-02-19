#include "http_srv.hpp"

#include <sstream>
#include <fstream>
#include <iterator>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <boost/log/trivial.hpp>
#include <boost/regex.hpp>


namespace http_srv {

  std::vector<std::string>
  http_srv::_parse_http_req(const std::string &request)
  {
    std::stringstream ss(request);
    std::vector<std::string> parsed_req;

    std::string str_req;

    while(std::getline(ss, str_req, '\n'))
      {
        if(str_req[str_req.length() - 1] == '\r')
          str_req.erase(str_req.length() - 1,1);

        parsed_req.emplace_back(str_req);
      }

    return parsed_req;
  }


  std::vector<std::string>
  http_srv::_parse_http_start_str(const std::string& request)
  {
    boost::regex regexp("(\\w+) /(\\S*) (HTTP/\\S+)");
    boost::smatch match_result;

    if((boost::regex_match(request, match_result, regexp))
       && (match_result.size() == 4))
      {
        std::vector<std::string> parsed_start_str;

        for(uint8_t i = 1; i < match_result.size(); ++i)
          parsed_start_str.emplace_back(match_result[i]);

        return parsed_start_str;
      }
    else
      {
        return std::vector<std::string>();
      }
  }


  std::string
  http_srv::_handle_http_get(const std::vector<std::string>& request)
  {
    std::vector<std::string> start_str = _parse_http_start_str(request[0]);

    if (start_str.size() == 3)
      {
        if(start_str[2] != "HTTP/1.1" && start_str[2] != "HTTP/1.0")
          {
            return _response_400();
          }

        if(_validate_uri(start_str[1]))
          {
            return _response_200(start_str[2] ,start_str[1]);
          }
        else
          {
            return _response_404(start_str[2]);
          }
      }
    else
      {
        return _response_400();
      }
  }


  bool
  http_srv::_validate_uri(std::string& path)
  {
    if(path.find("?") != std::string::npos)
      path.erase(path.begin() + path.find("?"), path.end());

    if(path.empty())
      {
        if(access("index.html", R_OK) == 0)
          {
            path = "index.html";
            return true;
          }
      }
    else if(access(path.c_str(), R_OK) == 0)
      return true;

    return false;
  }


  std::string
  http_srv::_response_200(const std::string &proto, std::string path)
  {
    std::string response(proto + " 200 OK\r\n");

    if(path.find("?") != std::string::npos)
      path.erase(path.begin() + path.find("?"), path.end());

    struct stat stat_buf;
    stat(path.c_str(), &stat_buf);

    std::ifstream file(path);

    response += std::string("Server: myserv\r\nContent-Type: text/html\r\nContent-Length: ")
        + std::to_string(stat_buf.st_size)
        + std::string("\r\n\r\n")
        + std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    file.close();
    return response;
  }


  void
  http_srv::_handler(int fd, tcp_srv::tcp_srv *serv, void *args)
  {
    http_srv *http_serv = (http_srv *)args;

    auto message = std::make_shared<std::vector<uint8_t> >();
    serv->tcp_recv(fd, message);
    std::string message_str(message->begin(), message->end());

    std::vector<std::string> http_req = http_serv->_parse_http_req(message_str);
    std::vector<std::string> start_str = http_serv->_parse_http_start_str(http_req[0]);

    std::string response;
    if (start_str.size() && start_str[0] == "GET")
      {
        response = http_serv->_handle_http_get(http_req);
      }
    else
      {
        response = http_serv->_response_400();
        return;
      }
    auto response_message = std::make_shared<std::vector<uint8_t> >(response.begin(),
                                                                    response.end());
    serv->tcp_send(fd, response_message);

    serv->delete_connection(fd);

    return;
  }


  void
  http_srv::start()
  {
    _tcp_serv.set_handler(&_handler, this);
    _tcp_serv.start();

    return;
  }

}
