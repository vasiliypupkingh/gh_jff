#pragma once

#include <string>
#include <stdint.h>


namespace options
{
  class Options
  {
  public:
    enum flag {
      ip_flag = 0x01,
      port_flag = 0x02,
      dir_flag = 0x04,
      log_flag = 0x08,
      daemon_flag = 0x10
    };


    void parse_opt(int argc, char *argv[]);
    uint16_t get_port() const;
    std::string get_ip() const;
    std::string get_dir() const;
    std::string get_log() const;
    bool is_daemon() const;
    bool is_log() const;
    uint8_t get_flags() const;
    void static print_help();

  private:
    uint16_t _port;
    std::string _ip;
    std::string _dir;
    std::string _log;
    uint8_t _flags = 0;
  };
}
