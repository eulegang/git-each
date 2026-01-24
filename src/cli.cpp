#include <charconv>
#include <initializer_list>
#include <iostream>
#include <ostream>
#include <string_view>
#include <thread>

#include "cli.h"

enum class ProcState {
  First,
  Blank,
  Dir,
  Jobs,
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
      } else if (str == "--jobs" || str == "-j") {
        state = ProcState::Jobs;
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

    case ProcState::Jobs:
      int res;
      const auto result =
          std::from_chars(str.data(), str.data() + str.size(), res);

      if (result.ec == std::errc{} && result.ptr == str.data() + str.size()) {
        cli->jobs = res;
      } else {
        cli->jobs = std::thread::hardware_concurrency();
      }
      state = ProcState::Blank;
    }
  }

  void finalize() {
    if (state == ProcState::Jobs) {
      cli->jobs = std::thread::hardware_concurrency();
      state = ProcState::Blank;
    }
  }
};

Cli::Cli(int argc, char *argv[])
    : args{}, help{}, version{}, system{}, discover{}, jobs{1},
      dir{std::nullopt} {
  Processor proc(this);

  for (int i = 0; i < argc; i++) {
    proc.accept(argv[i]);
  }

  proc.finalize();
}

Cli::Cli(std::initializer_list<std::string_view> args)
    : args{}, help{}, version{}, system{}, discover{}, jobs{1},
      dir{std::nullopt} {
  Processor proc(this);

  for (const auto &arg : args) {
    proc.accept(arg);
  }

  proc.finalize();
}

bool Cli::validate() const {
  if (args.empty() && !discover) {
    std::cerr << "no command was given" << std::endl;
    return false;
  }

  if (jobs < 1) {
    std::cerr << "invalid jobs specified" << std::endl;
    return false;
  }

  return true;
}
