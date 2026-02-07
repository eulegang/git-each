
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <thread>

#include "cli.h"
#include "cmd.h"
#include "comms.h"
#include "discover.h"
#include "format.h"
#include "tempfile.h"
#include "worker.h"

using path_t = std::filesystem::path;

std::string resolve_path(std::filesystem::path);

int main(int argc, char *argv[]) {
  try {
    Cli cli(argc, argv);

    if (cli.help) {
      std::cout
          << "git each [-s | --system] [-V | --version] [-d | --directory "
             "<dir>] <cmd>"
          << std::endl;
      return 0;
    }

    if (!cli.validate()) {
      return 1;
    }

    path_t start_path = cli.dir ? *cli.dir : std::filesystem::current_path();

    if (cli.discover) {
      discover(start_path,
               [](auto path) { std::cout << path.string() << std::endl; });
      return 0;
    }

    Formatter formatter{cli.format, start_path};

    std::string file_backing;
    std::vector<std::string_view> args;
    if (cli.system) {
      args = std::move(cli.args);
    } else if (cli.file) {
      const char *interp = getenv("SHELL");
      if (cli.interpreter) {
        interp = cli.interpreter->c_str();
      }

      file_backing = resolve_path(*cli.file);

      args.push_back(interp);
      args.push_back(file_backing);

      for (const auto &arg : cli.args) {
        args.push_back(arg);
      }
    } else {
      args.push_back("git");
      for (const auto &arg : cli.args) {
        args.push_back(arg);
      }
    }

    FanOut<path_t> to_workers;
    FanIn<std::shared_ptr<CmdOutput>> to_report;
    TempBase tmp_base("git-each");

    Cmd cmd(args, &tmp_base);

    std::vector<std::thread> threads;
    for (int i = 0; i < cli.jobs; i++) {
      threads.push_back(
          std::thread(worker, to_workers.recv(), &cmd, to_report.sender()));
    }

    discover(start_path, [&to_workers](auto path) { to_workers.push(path); });
    to_workers.close();

    report(to_report, formatter);

    for (auto &th : threads) {
      th.join();
    }
  } catch (FormatterException &e) {
    for (const auto &detail : e.details) {
      std::cerr << "  - " << detail << std::endl;
    }
    return 2;
  } catch (std::system_error &e) {
    std::cerr << e.what() << std::endl;
    std::cerr << e.code().message() << std::endl;
    return 2;
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 2;
  }
}

std::string resolve_path(std::filesystem::path path) {
  if (!is_executable(path))
    throw std::runtime_error("unable to resolve command");

  if (path.has_root_path()) {
    return path.string();
  } else {
    std::filesystem::path base = std::filesystem::current_path();
    return (base / path).lexically_normal().string();
  }
}
