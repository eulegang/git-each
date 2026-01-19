#ifndef _GIT_EACH_CLI_H
#define _GIT_EACH_CLI_H

#include <filesystem>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <vector>

struct Cli {
  std::vector<std::string_view> args;
  bool help;
  bool version;
  bool system;
  bool discover;
  std::optional<std::filesystem::path> dir;

  Cli(int argc, char *argv[]);
  Cli(std::initializer_list<std::string_view>);
};

std::ostream &operator<<(std::ostream &os, const Cli &cli);

#endif
