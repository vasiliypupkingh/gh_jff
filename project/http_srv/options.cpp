#include <getopt.h>
#include <iostream>

#include "options.h"


namespace options {
  void
  Options::parse_opt(int argc, char *argv[])
  {
    int option;
    char* ip_str = nullptr;

    while ((option = getopt(argc, argv, "i:p:r:l:dh")) != -1)
      {
        switch (option)
          {
          case 'i':
            ip_str = optarg;
            _flags |= flag::ip_flag;
            break;
          case 'p':
            _port = atoi(optarg);
            _flags |= flag::port_flag;
            break;
          case 'r':
            _dir = std::string(optarg);
            _flags |= flag::dir_flag;
            break;
          case 'l':
            _log = std::string(optarg);
            _flags |= flag::log_flag;
            break;
          case 'd':
            _flags |= flag::daemon_flag;
            break;
          case 'h':
            print_help();
            exit(EXIT_SUCCESS);
            break;

          default:
            print_help();
            exit(EXIT_FAILURE);
            break;
          }
      }

    if(_flags && flag::ip_flag)
      _ip = std::string(ip_str);
    return;
  }


  uint16_t
  Options::get_port() const
  {
    return _port;
  }


  std::string
  Options::get_ip() const
  {
    return _ip;
  }


  std::string
  Options::get_dir() const
  {
    return _dir;
  }


  std::string
  Options::get_log() const
  {
    return _log;
  }


  bool
  Options::is_daemon() const
  {
    return static_cast<bool>(_flags & flag::daemon_flag);
  }


  bool
  Options::is_log() const
  {
    return static_cast<bool>(_flags & flag::log_flag);
  }


  uint8_t
  Options::get_flags() const
  {
    return _flags;
  }


  void
  Options::print_help()
  {
    std::cerr << "Usage:" <<std::endl
              << " http_serv [-i ip] [-p port] [-r dir] [-l log] [-d daemon mode] [-h help]"
              << std::endl;
  }
}
