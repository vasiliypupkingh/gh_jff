#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


struct options {
  enum flag {
    out_file_flag = 0x01,
    command_flag = 0x02,
  };
  std::string out_file;
  std::string command;
  uint8_t flags = 0;
};


static
void
parse_opt(int argc, char *argv[], options& opt)
{
  int option;

  while ((option = getopt(argc, argv, "c:o:h")) != -1)
    {
      switch (option)
      {
      case 'c':
          opt.command = optarg;
          opt.flags |= options::flag::command_flag;
          break;

      case 'o':
          opt.out_file = optarg;
          opt.flags |= options::flag::out_file_flag;
          break;

        case 'h':
          fprintf(stderr, "Usage: %s [-c command] [-o output file]\n", argv[0]);
          exit(EXIT_SUCCESS);
        default:
          fprintf(stderr, "Usage: %s [-c command] [-o output file]\n", argv[0]);
          exit(EXIT_FAILURE);
      }
    }

  return;
}


static
void
parse(string& str_com, vector<vector<string>>& commands)
{
  stringstream commands_stream(str_com);
  string command;
  while(getline(commands_stream, command, '|'))
    {
      vector<string> vector_of_args;
      stringstream comand_stream(command);

      while(comand_stream.peek() == ' ')
        comand_stream.ignore();

      string arg;
      while(getline(comand_stream, arg, ' '))
        vector_of_args.emplace_back(arg);

      commands.emplace_back(vector_of_args);
    }
}


static
void
exec(vector<string>& command)
{
  char **const argv_v = new char* [command.size() + 1];
  for(size_t i = 0; i < command.size(); ++i)
    {
      argv_v[i] = new char [command[i].length()];
      strcpy(argv_v[i], command[i].c_str());
    }
  argv_v[command.size()] = NULL;

  execvp(argv_v[0], argv_v);

  for(size_t i = 0; i < command.size(); ++i)
    {
      delete[] argv_v[i];
    }

  return;
}


static
void
fork_and_exec(size_t number_of_command, vector<vector<string>>& commands)
{
  if(number_of_command > commands.size())
    return;

  if(number_of_command == commands.size())
    {
      exec(commands[commands.size() - number_of_command]);
    }
  else
    {
      int pfd[2];
      pipe(pfd);
      pid_t child_pid = fork();

      if(child_pid)
        {
          close(STDIN_FILENO);
          dup2(pfd[0], STDIN_FILENO);
          close(pfd[0]);
          close(pfd[1]);
          exec(commands[commands.size() - number_of_command]);
        }
      else
        {
          close(STDOUT_FILENO);
          dup2(pfd[1], STDOUT_FILENO);
          close(pfd[0]);
          close(pfd[1]);
          fork_and_exec(number_of_command + 1, commands);
        }
    }
}


int
main(int argc, char *argv[])
{
  //---------parse of a input string and options-------
  vector<vector<string>> commands;
  string str_com;

  options opt;
  parse_opt(argc, argv, opt);

  if(opt.flags & (options::flag::command_flag))
    str_com = opt.command;
  else
    getline(cin, str_com, '\n');

  parse(str_com, commands);
  //---------fork and exec-----------------------------
  if(opt.flags & (options::flag::out_file_flag))
    freopen(opt.out_file.c_str(), "w", stdout);

  fork_and_exec(1, commands);

  fclose(stdout);
  //---------------------------------------------------
  return 0;
}
