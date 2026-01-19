#include "cli.h"
#include <iostream>

int main(int argc, char *argv[]) {
  Cli cli(argc, argv);

  if (cli.help) {
    std::cout << "git each [-s | --system] [-V | --version] [-d | --directory "
                 "<dir>] <cmd>"
              << std::endl;
    return 0;
  }

  if (cli.args.empty()) {
    std::cerr << "no command was given" << std::endl;
    return 1;
  }

  std::cout << cli << std::endl;
}
