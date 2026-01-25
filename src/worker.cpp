#include "worker.h"
#include "cmd.h"
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

    formatter.format(*output, 1);
  }
}
