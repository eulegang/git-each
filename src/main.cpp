
#include <filesystem>
#include <iostream>

#include "cli.h"
#include "comms.h"
#include "discover.h"
#include "tempfile.h"
#include "worker.h"

using path_t = std::filesystem::path;

int main(int argc, char *argv[]) {
  Cli cli(argc, argv);

  if (cli.help) {
    std::cout << "git each [-s | --system] [-V | --version] [-d | --directory "
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

  std::vector<std::string_view> args;
  if (!cli.system) {
    args.push_back("git");
    for (const auto &arg : cli.args) {
      args.push_back(arg);
    }
  } else {
    args = std::move(cli.args);
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

  report(to_report);

  for (auto &th : threads) {
    th.join();
  }
}
