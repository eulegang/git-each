#include "worker.h"
#include "cmd.h"

#include <iostream>

void worker(FanOut<std::filesystem::path>::Recv recv, Cmd *cmd) {
  std::optional<std::filesystem::path> input;
  while ((input = recv.recv())) {
    std::filesystem::path cur = *input;

    CmdOutput out = cmd->run(cur);

    std::cout << "worker: " << cur.string() << std::endl;
  }
}
