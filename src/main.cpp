
#include <filesystem>
#include <iostream>

#include "cli.h"
#include "comms.h"
#include "discover.h"

using path_t = std::filesystem::path;

std::thread spawn_worker(FanOut<path_t>::Recv recv);
void worker(FanOut<path_t>::Recv recv);

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

  FanOut<path_t> to_workers;

  std::vector<std::thread> threads;
  for (int i = 0; i < 4; i++) {
    threads.push_back(std::thread(worker, to_workers.recv()));
  }

  discover(start_path, [&to_workers](auto path) { to_workers.push(path); });
  to_workers.close();

  for (auto &th : threads) {
    th.join();
  }
}

void worker(FanOut<path_t>::Recv recv) {
  std::optional<path_t> input;
  while ((input = recv.recv())) {
    path_t cur = *input;

    std::cout << "worker: " << cur.string() << std::endl;
  }
}
