
#include "cmd.h"

#include <algorithm>
#include <filesystem>
#include <grp.h>
#include <memory>
#include <pwd.h>
#include <ranges>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

std::vector<gid_t> my_groups(uid_t uid) {
  static std::vector<gid_t> groups;

  if (!groups.empty()) {
    return groups;
  }

  struct passwd *passwd = getpwuid(uid);
  std::string_view username{passwd->pw_name};
  std::vector<gid_t> my_groups;

  setgrent();
  struct group *grp;
  while ((grp = getgrent())) {
    char **members = grp->gr_mem;
    while (*members) {
      if (*members == username) {
        groups.push_back(grp->gr_gid);
      }

      members++;
    }
  }

  endgrent();

  return groups;
}

std::string resolve(std::string_view prog) {
  if (prog.starts_with("/")) {
    if (!is_executable(prog))
      throw std::runtime_error("unable to resolve command");

    return std::string(prog);
  }

  if (prog.starts_with("./") || prog.starts_with("../")) {
    std::filesystem::path base = std::filesystem::current_path();
    std::filesystem::path cur(prog);
    std::filesystem::path path = base / prog;

    if (!is_executable(path)) {
      throw std::runtime_error("unable to resolve command");
    }

    return std::string(path);
  }

  std::filesystem::path prog_path{prog};
  std::string_view view{getenv("PATH")};
  constexpr std::string_view delim{":"};
  for (const auto &range : view | std::views::split(delim)) {
    std::string_view dir{range.data(), range.size()};
    std::filesystem::path env_path{dir};

    std::filesystem::path exec_path = env_path / prog_path;

    if (is_executable(exec_path)) {
      return exec_path.string();
    }
  }

  throw std::runtime_error("unable to resolve command");
}

Cmd::Args::Args(std::vector<std::string_view> args) {

  if (args.size() >= 64)
    throw std::runtime_error("invariant: can't handle over 64 arguments");

  buffer = new char[8192];
  offsets = new char *[64];
  int buffer_index = 0, offset_index = 0;

  for (const auto &arg : args) {
    memcpy(buffer + buffer_index, arg.data(), arg.size());
    buffer[buffer_index + arg.size()] = 0;

    offsets[offset_index++] = buffer + buffer_index;
    buffer_index += arg.size() + 1;
  }

  offsets[offset_index] = 0;
}

Cmd::Args::~Args() {
  delete[] buffer;
  delete[] offsets;
}

Cmd::Cmd(std::vector<std::string_view> args, TempBase *base)
    : tmp_base{base}, args{args} {

  if (args.size() == 0) {
    throw std::runtime_error("invalid command");
  }

  resolved = resolve(args[0]);
}

std::shared_ptr<CmdOutput> Cmd::run(std::filesystem::path dir) {
  TempInst inst = tmp_base->inst();

  auto out = inst.tmp("stdout");
  auto err = inst.tmp("stderr");
  int status;

  pid_t pid = fork();

  if (pid == -1) {
    throw std::system_error(errno, std::system_category(),
                            "failed to fork process");
  }

  if (pid == 0) {
    out->dup(1);
    err->dup(2);
    int res = chdir(dir.c_str());
    if (res == -1) {
      throw std::system_error(errno, std::system_category(),
                              "failed to chdir into directory");
      ;
    }

    Args args(this->args);
    res = execv(this->resolved.c_str(), args.offsets);

    if (res == -1) {
      throw std::system_error(errno, std::system_category(),
                              "failed to execute command");
    }
  } else {
    pid_t res = waitpid(pid, &status, 0);

    if (res == -1) {
      throw std::system_error(errno, std::system_category(),
                              "failed to wait for child");
    }
  }

  return std::make_shared<CmdOutput>(status, dir, std::move(out),
                                     std::move(err));
}

bool is_executable(std::filesystem::path path) {
  if (!std::filesystem::is_regular_file(path)) {
    return false;
  }

  struct stat stat_value;
  if (stat(path.c_str(), &stat_value) == -1) {
    return false;
  }

  const uid_t uid = getuid();
  const std::vector<gid_t> gids = my_groups(uid);

  const bool user =
      (stat_value.st_mode & S_IXUSR) != 0 && stat_value.st_uid == uid;

  const bool grp =
      (stat_value.st_mode & S_IXGRP) != 0 &&
      std::find(gids.begin(), gids.end(), stat_value.st_gid) == gids.end();

  const bool world = (stat_value.st_mode & S_IXOTH) != 0;

  return user || grp || world;
}
