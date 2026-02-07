#ifndef _GIT_EACH_CMD_H
#define _GIT_EACH_CMD_H

#include <filesystem>
#include <string_view>
#include <vector>

#include "tempfile.h"

struct CmdOutput {
  int status;
  std::filesystem::path cwd;
  std::unique_ptr<TempFile> out;
  std::unique_ptr<TempFile> err;

  CmdOutput(int status, std::filesystem::path cwd,
            std::unique_ptr<TempFile> out, std::unique_ptr<TempFile> err)
      : status{status}, cwd{cwd}, out{std::move(out)}, err{std::move(err)} {}
};

class Cmd {
  TempBase *tmp_base;
  std::vector<std::string_view> args;
  std::string resolved;

  struct Args {
    char *buffer;
    char **offsets;
    Args(std::vector<std::string_view> args);
    ~Args();
  };

public:
  Cmd(std::vector<std::string_view> args, TempBase *base);
  std::shared_ptr<CmdOutput> run(std::filesystem::path dir);
};

bool is_executable(std::filesystem::path path);

#endif
