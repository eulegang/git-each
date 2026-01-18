#include "cli.h"
#include <filesystem>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <string_view>

enum class ProcState {
  Blank,
  Dir,
  Fused,
};

struct Processor {
  std::vector<std::string_view> args;
  bool help;
  bool version;
  std::filesystem::path dir;

  bool first;
  ProcState state;

  Processor() {
    first = true;
    help = false;
    version = false;
    state = ProcState::Blank;
  }

  void accept(std::string_view str) {
    if (first) {
      first = false;
      return;
    }

    switch (state) {
    case ProcState::Blank:
      if (str == "--help" || str == "-h") {
        help = true;
      } else if (str == "--version" || str == "-V") {
        version = true;
      } else if (str == "--directory" || str == "-d") {
        state = ProcState::Dir;
      } else if (str == "--") {
        state = ProcState::Fused;
      } else {
        args.push_back(str);
      }
      break;
    case ProcState::Dir:
      dir = str;

      state = ProcState::Blank;
      break;
    case ProcState::Fused:
      args.push_back(str);
      break;
    }
  }

  void finalize(Cli *cli) {
    cli->args = std::move(args);
    cli->help = help;
    cli->version = version;
    cli->dir = dir;
  }
};

Cli::Cli(int argc, char *argv[]) {
  Processor proc;

  for (int i = 0; i < argc; i++) {
    proc.accept(argv[i]);
  }

  proc.finalize(this);
}

Cli::Cli(std::initializer_list<std::string_view> args) {
  Processor proc;

  for (const auto &arg : args) {
    proc.accept(arg);
  }

  proc.finalize(this);
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
