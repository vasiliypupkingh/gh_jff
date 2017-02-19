#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

#include <thread>
#include <string>

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>

#include "tcp_srv.hpp"
#include "http_srv.hpp"
#include "options.h"


static
void
signals_handler(int signum)
{
  if (signum == SIGINT || signum == SIGTERM)
    {
      std::cerr << "Recive a signal " << signum << std::endl
                << "Exit" << std::endl;
      exit(EXIT_SUCCESS);
    }
  return;
}


static
void
init_log(const std::string& log_file)
{
  boost::log::add_file_log(boost::log::keywords::file_name = log_file,
                           boost::log::keywords::auto_flush = true);
  BOOST_LOG_TRIVIAL(debug) << "Start log";
  return;
}


static
void
start_srv(std::string ipv4_str, uint16_t port, std::string root)
{
  http_srv::http_srv serv(ipv4_str, port, root);
  serv.start();
  return;
}


int
main(int argc, char *argv[])
{
  signal(SIGINT, signals_handler);
  signal(SIGTERM, signals_handler);

  options::Options option;
  option.parse_opt(argc, argv);

  auto option_flags = option.get_flags();

  if (!(option_flags && options::Options::flag::ip_flag)
      || !(option_flags && options::Options::flag::port_flag)
      || !(option_flags && options::Options::flag::dir_flag))
    {
      options::Options::print_help();
      return 1;
    }

  if (option.is_log())
    init_log(option.get_log());
  if(option.is_daemon())
    {
      if (daemon(1, 0) != 0)
        {
          std::cerr << "Can't demonize the process" << std::endl;
          return 1;
        }
    }

  std::thread thread_srv(start_srv, option.get_ip(), option.get_port(), option.get_dir());
  thread_srv.join();

  return 0;
}

