#include "cli.h"
#include <initializer_list>
#include <ostream>
#include <string_view>

struct Processor {
  std::vector<std::string_view> args;
  bool help;
  bool version;

  bool first;

  Processor() {
    first = true;
    help = false;
    version = false;
  }

  void accept(std::string_view str) {
    if (first) {
      first = false;
      return;
    }

    if (str == "--help" || str == "-h") {
      help = true;
    } else if (str == "--version" || str == "-V") {
      version = true;
    } else {
      args.push_back(str);
    }
  }

  void finalize(Cli *cli) {
    cli->args = std::move(args);
    cli->help = help;
    cli->version = version;
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
  os << "}" << std::endl;

  return os;
}
