#ifndef _GIT_EACH_FMT_H
#define _GIT_EACH_FMT_H

#include "cmd.h"
#include <exception>
#include <string_view>

struct Op {
  enum : size_t {
    StrTab,
    Status,
    Cwd,
    Emit,
    CheckSuccess,
    CheckFailure,
    CheckStream,
    GroupBegin,
    GroupEnd,
  } code;

  size_t arg;

  static Op strtab(size_t index) { return Op{.code = StrTab, .arg = index}; }
  static Op status() { return Op{.code = Status, .arg = 0}; }
  static Op cwd() { return Op{.code = Cwd, .arg = 0}; }
  static Op emit(size_t index) { return Op{.code = Emit, .arg = index}; }
  static Op check_success() { return Op{.code = CheckSuccess, .arg = 0}; }
  static Op check_failure() { return Op{.code = CheckFailure, .arg = 0}; }
  static Op check_stream(size_t index) {
    return Op{.code = CheckStream, .arg = index};
  }
  static Op begin() { return Op{.code = GroupBegin, .arg = 0}; }
  static Op end() { return Op{.code = GroupEnd, .arg = 0}; }
};

std::ostream &operator<<(std::ostream &, const Op &);
bool operator==(const Op a, const Op b);

struct Formatter {
  std::vector<std::string_view> strtab;
  std::vector<Op> bytecode;

  Formatter(std::string_view);
  void format(CmdOutput &, int fd);
};

class FormatterException : public std::exception {
public:
  std::string title;
  std::vector<std::string> details;

  FormatterException(std::string title, std::vector<std::string> details)
      : title{title}, details{details} {};
  const char *what() const noexcept override { return title.c_str(); };
};

#endif
