#include "worker.h"
#include "cmd.h"

#include <iostream>
#include <memory>

void worker(FanOut<std::filesystem::path>::Recv recv, Cmd *cmd,
            FanIn<std::shared_ptr<CmdOutput>>::Sender report) {
  std::optional<std::filesystem::path> input;
  while ((input = recv.recv())) {
    std::filesystem::path cur = *input;

    std::shared_ptr<CmdOutput> out = cmd->run(std::move(cur));
    report.push(out);
  }

  report.close();
}

void report(FanIn<std::shared_ptr<CmdOutput>> &in, Formatter &formatter) {

  std::optional<std::shared_ptr<CmdOutput>> cmd;

  while ((cmd = in.recv())) {
    std::shared_ptr<CmdOutput> output = *cmd;

    formatter.format(*output);

    std::cout << "worker: " << output->cwd.string() << std::endl;
    std::cout << "status: " << output->status << std::endl;
    std::cout << "begin-stdout" << std::endl;
    output->out->dump(1);
    std::cout << "end-stdout" << std::endl;
    std::cerr << "begin-stderr" << std::endl;
    output->err->dump(2);
    std::cerr << "end-stderr" << std::endl;
  }
}
