#ifndef _GIT_EACH_CMD_H
#define _GIT_EACH_CMD_H

#include <filesystem>
#include <string_view>
#include <vector>

#include "tempfile.h"

struct CmdOutput {
  int status;
  std::unique_ptr<TempFile> out;
  std::unique_ptr<TempFile> err;
};

class Cmd {
  TempBase *tmp_base;
  std::vector<std::string_view> args;

public:
  Cmd(std::vector<std::string_view> args, TempBase *base);
  CmdOutput run(std::filesystem::path dir);
};

#endif
