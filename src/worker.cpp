#include "worker.h"
#include "cmd.h"

#include <iostream>

void worker(FanOut<std::filesystem::path>::Recv recv, Cmd *cmd) {
  std::optional<std::filesystem::path> input;
  while ((input = recv.recv())) {
    std::filesystem::path cur = *input;

    CmdOutput out = cmd->run(cur);

    std::cout << "worker: " << cur.string() << std::endl;
    std::cout << "status: " << out.status << std::endl;
    std::cout << "begin-stdout" << std::endl;
    out.out->dump(1);
    std::cout << "end-stdout" << std::endl;
    std::cerr << "begin-stderr" << std::endl;
    out.err->dump(2);
    std::cerr << "end-stderr" << std::endl;
  }
}
