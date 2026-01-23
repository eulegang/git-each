#include "worker.h"
#include "cmd.h"

#include <iostream>
#include <memory>

void worker(FanOut<std::filesystem::path>::Recv recv, Cmd *cmd,
            FanIn<std::shared_ptr<CmdOutput>>::Sender report) {
  std::cout << "starting worker" << std::endl;
  std::optional<std::filesystem::path> input;
  while ((input = recv.recv())) {
    std::filesystem::path cur = *input;

    std::shared_ptr<CmdOutput> out = cmd->run(cur);
    report.push(out);
  }

  std::cout << "closing worker" << std::endl;

  report.close();
}

void report(FanIn<std::shared_ptr<CmdOutput>> &in) {

  std::optional<std::shared_ptr<CmdOutput>> cmd;

  while ((cmd = in.recv())) {
    std::shared_ptr<CmdOutput> output = *cmd;

    // std::cout << "worker: " << cur.string() << std::endl;
    std::cout << "status: " << output->status << std::endl;
    std::cout << "begin-stdout" << std::endl;
    output->out->dump(1);
    std::cout << "end-stdout" << std::endl;
    std::cerr << "begin-stderr" << std::endl;
    output->err->dump(2);
    std::cerr << "end-stderr" << std::endl;
  }
}
