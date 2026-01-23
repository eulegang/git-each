
#include "cmd.h"

#include <format>
#include <memory>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

struct Args {
  char *buffer;
  char **offsets;
  Args(std::vector<std::string_view> args) {

    if (args.size() >= 64)
      throw "invariant: can't handle over 64 arguments";

    buffer = new char[8192];
    offsets = new char *[64];
    int buffer_index = 0, offset_index = 0;

    for (const auto &arg : args) {
      memcpy(buffer + buffer_index, arg.data(), arg.size());
      buffer[buffer_index + arg.size()] = 0;

      offsets[offset_index++] = buffer + buffer_index;
      buffer_index += arg.size() + 1;
    }
  }

  ~Args() {
    delete[] buffer;
    delete[] offsets;
  }
};

Cmd::Cmd(std::vector<std::string_view> args, TempBase *base)
    : tmp_base{base}, args{args} {

  if (args.size() == 0) {
    throw "invalid command";
  }
}

CmdOutput Cmd::run(std::filesystem::path dir) {
  TempInst inst = tmp_base->inst();

  auto out = inst.tmp("stdout");
  auto err = inst.tmp("stderr");
  int status;

  pid_t pid = fork();

  if (pid == -1) {
    throw "failed to fork child";
  }

  if (pid == 0) {
    out->dup(1);
    err->dup(2);
    int res = chdir(dir.c_str());
    if (res == -1) {
      throw "failed to chdir into directory";
    }

    Args args(this->args);
    res = execv(this->args[0].data(), args.offsets);

    if (res == -1) {
      int e = errno;
      throw std::format("failed to exec new process: {}", e);
    }
  } else {
    pid_t res = waitpid(pid, &status, 0);

    if (res == -1) {
      throw "failed to wait for child"; // TODO: figure out an exception
    }
  }

  return CmdOutput{
      .status = status,
      .out = std::move(out),
      .err = std::move(err),
  };
}
