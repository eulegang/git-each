#include "cli.h"
#include <iostream>

int main(int argc, char *argv[]) {
  Cli cli(argc, argv);

  std::cout << cli << std::endl;
}
