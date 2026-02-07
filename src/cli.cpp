#include <charconv>
#include <initializer_list>
#include <iostream>
#include <optional>
#include <ostream>
#include <string_view>
#include <thread>

#include "cli.h"

static const std::string_view DEFAULT_FORMAT = "%c%(%s%)%n%O%o%E%e";

enum class ProcState {
  First,
  Blank,
  Dir,
  Jobs,
  Fused,
  Format,
  File,
  Interp,
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
      } else if (str == "--format" || str == "-f") {
        state = ProcState::Format;
      } else if (str == "--file" || str == "-F") {
        state = ProcState::File;
      } else if (str == "--interpreter" || str == "-I") {
        state = ProcState::Interp;
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

    case ProcState::Format:
      cli->format = str;
      state = ProcState::Blank;
      break;

    case ProcState::File:
      cli->file = str;
      state = ProcState::Blank;
      break;

    case ProcState::Interp:
      cli->interpreter = str;
      state = ProcState::Blank;
      break;

    case ProcState::Jobs:
      int res;
      const auto result =
          std::from_chars(str.data(), str.data() + str.size(), res);

      if (result.ec == std::errc{} && result.ptr == str.data() + str.size()) {
        cli->jobs = res;
      } else {
        return;
      }
      state = ProcState::Blank;
      break;
    }
  }

  void finalize() {}
};

Cli::Cli(int argc, char *argv[])
    : args{}, help{}, version{}, system{}, discover{}, format{DEFAULT_FORMAT},
      dir{std::nullopt}, file{std::nullopt}, interpreter{std::nullopt} {
  jobs = std::thread::hardware_concurrency();

  Processor proc(this);

  for (int i = 0; i < argc; i++) {
    proc.accept(argv[i]);
  }

  proc.finalize();
}

Cli::Cli(std::initializer_list<std::string_view> args)
    : args{}, help{}, version{}, system{}, discover{}, format{DEFAULT_FORMAT},
      dir{std::nullopt}, file{std::nullopt}, interpreter{std::nullopt} {
  jobs = std::thread::hardware_concurrency();

  Processor proc(this);

  for (const auto &arg : args) {
    proc.accept(arg);
  }

  proc.finalize();
}

bool Cli::validate() const {
  if (args.empty() && !discover && !file) {
    std::cerr << "no command was given" << std::endl;
    return false;
  }

  if (jobs < 1) {
    std::cerr << "invalid jobs specified" << std::endl;
    return false;
  }

  if (interpreter && !file) {
    std::cerr << "can not use an interpreter without a file to interpret"
              << std::endl;
  }

  if ((interpreter || file) && system) {
    std::cerr << "can not run file with system flag" << std::endl;
  }

  return true;
}
