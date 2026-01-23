
#ifndef _GIT_EACH_WORKER_H
#define _GIT_EACH_WORKER_H

#include "cmd.h"
#include "comms.h"
#include <filesystem>

void worker(FanOut<std::filesystem::path>::Recv, Cmd *,
            FanIn<std::shared_ptr<CmdOutput>>::Sender);

void report(FanIn<std::shared_ptr<CmdOutput>> &);

#endif
