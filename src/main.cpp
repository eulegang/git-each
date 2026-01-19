
#include <filesystem>
#include <iostream>

#include "cli.h"
#include "discover.h"

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

  std::filesystem::path start_path =
      cli.dir ? *cli.dir : std::filesystem::current_path();

  discover(start_path,
           [](auto path) { std::cout << "repo found " << path << std::endl; });
}
