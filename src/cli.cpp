#include "cli.h"
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <string_view>

enum class ProcState {
  First,
  Blank,
  Dir,
  Fused,
};

struct Processor {
  Cli *cli;

  ProcState state;

  Processor(Cli *cli) : cli{cli} { state = ProcState::First; }

  void accept(std::string_view str) {
    switch (state) {
    case ProcState::First:
      state = ProcState::Blank;
      break;
    case ProcState::Blank:
      if (str == "--help" || str == "-h") {
        cli->help = true;
      } else if (str == "--version" || str == "-V") {
        cli->version = true;
      } else if (str == "--directory" || str == "-d") {
        state = ProcState::Dir;
      } else if (str == "--system" || str == "-s") {
        cli->system = true;
      } else if (str == "--discover" || str == "-D") {
        cli->discover = true;
      } else if (str == "--") {
        state = ProcState::Fused;
      } else {
        cli->args.push_back(str);
      }
      break;
    case ProcState::Dir:
      cli->dir = str;

      state = ProcState::Blank;
      break;
    case ProcState::Fused:
      cli->args.push_back(str);
      break;
    }
  }
};

Cli::Cli(int argc, char *argv[])
    : args{}, help{}, version{}, system{}, discover{}, dir{std::nullopt} {
  Processor proc(this);

  for (int i = 0; i < argc; i++) {
    proc.accept(argv[i]);
  }
}

Cli::Cli(std::initializer_list<std::string_view> args)
    : args{}, help{}, version{}, system{}, discover{}, dir{std::nullopt} {
  Processor proc(this);

  for (const auto &arg : args) {
    proc.accept(arg);
  }
}

std::ostream &operator<<(std::ostream &os, const Cli &cli) {
  os << "CLi {" << std::endl;
  os << "  args = <";
  for (const auto &arg : cli.args) {
    os << "\"" << arg << "\" ";
  }
  os << ">" << std::endl;

  os << "  help = " << cli.help << std::endl;
  os << "  version = " << cli.version << std::endl;
  os << "}" << std::endl;

  return os;
}
