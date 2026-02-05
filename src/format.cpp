#include "format.h"
#include "cmd.h"
#include <array>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <system_error>

bool operator==(const Op a, const Op b) {
  return a.code == b.code && a.arg == b.arg;
}

class Output {
  const size_t CAP = 4096;
  char *buf;
  size_t idx;
  int fd;

public:
  Output(int fd) : idx{0}, fd{fd} { buf = new char[CAP]; }
  ~Output() { delete[] buf; }

  void push(std::string_view str) {
    if (str.size() + idx > CAP) {
      flush();

      if (str.size() > CAP) {
        size_t cur = 0;

        while (cur != str.size()) {
          int bytes = write(fd, str.data() + cur, str.size());

          if (bytes == -1) {
            throw std::system_error();
          }

          cur += bytes;
        }

        write(fd, str.data(), str.size());
        return;
      }
    }

    memcpy(buf + idx, str.data(), str.size());
    idx += str.size();
  }

  void flush() {
    ssize_t bytes = write(fd, buf, idx);
    if (bytes == -1)
      throw std::system_error();

    if (bytes < (ssize_t)idx) {
      memmove(buf, buf + bytes, idx - bytes);
      idx -= bytes;
    } else {
      idx = 0;
    }
  }
};

std::ostream &operator<<(std::ostream &os, const Op &op) {
  switch (op.code) {
  case Op::StrTab:
    return os << "strtab(" << op.arg << ")";
  case Op::Status:
    return os << "status";
  case Op::Cwd:
    return os << "cwd";
  case Op::Emit:
    return os << "emit(" << op.arg << ")";
  case Op::CheckSuccess:
    return os << "check_success";
  case Op::CheckFailure:
    return os << "check_failure";
  case Op::CheckStream:
    return os << "check_stream(" << op.arg << ")";
  case Op::GroupBegin:
    return os << "begin";
  case Op::GroupEnd:
    return os << "end";
  default:
    return os << "unknown";
  }
}

void validate(Formatter *formatter) {
  std::vector<std::string> msg;
  int group_count = 0;

  for (const auto &op : formatter->bytecode) {
    switch (op.code) {
    case Op::StrTab:
      if (op.arg >= formatter->strtab.size()) {
        msg.push_back("invalid static string");
      }
      break;
    case Op::Emit:
      if (op.arg >= 2) {
        msg.push_back("emitting invalid stream");
      }
      break;
    case Op::CheckStream:
      if (op.arg >= 2) {
        msg.push_back("checking invalid stream");
      }
      break;
    case Op::GroupBegin:
      group_count++;
      break;
    case Op::GroupEnd:
      group_count--;
      if (group_count < 0) {
        msg.push_back("underflowed grouping");
      }
      break;
    case Op::Cwd:
      if (op.arg != 0 && op.arg != 1) {
        msg.push_back("invalid cwd argument");
      }
    case Op::CheckSuccess:
    case Op::CheckFailure:
    case Op::Status:
      break;
    }
  }

  if (group_count != 0) {
    msg.push_back("unbalanced group");
  }

  if (msg.size() > 0) {
    throw FormatterException("invalid format", msg);
  }
}

Formatter::Formatter(std::string_view fmt, std::filesystem::path base_dir)
    : base_dir{base_dir}, strtab{}, bytecode{} {
  size_t base = 0;
  bool interp = false;

  const auto push_strtab = [this](std::string_view view) {
    const size_t index = strtab.size();
    strtab.push_back(view);
    bytecode.push_back(Op::strtab(index));
  };

  const auto commit_strtab = [this, fmt, &base](size_t i) {
    if (base - i != 0) {
      const size_t index = this->strtab.size();
      this->strtab.push_back(fmt.substr(base, i - base));
      this->bytecode.push_back(Op::strtab(index));

      base = i;
    }
  };

  for (size_t i{}; i < fmt.size(); i++) {
    char ch = fmt[i];

    if (interp) {
      switch (ch) {
      case 's':
        bytecode.push_back(Op::status());
        break;

      case 'o':
        bytecode.push_back(Op::emit(0));
        break;

      case 'e':
        bytecode.push_back(Op::emit(1));
        break;

      case 'c':
        bytecode.push_back(Op::cwd(0));
        break;

      case 'C':
        bytecode.push_back(Op::cwd(1));
        break;

      case 'S':
        bytecode.push_back(Op::check_success());
        break;

      case 'F':
        bytecode.push_back(Op::check_failure());
        break;

      case 'O':
        bytecode.push_back(Op::check_stream(0));
        break;

      case 'E':
        bytecode.push_back(Op::check_stream(1));
        break;

      case '(':
        push_strtab("(");
        break;

      case ')':
        push_strtab(")");
        break;

      case 'n':
        push_strtab("\n");
        break;

      default:
        throw FormatterException(std::format("invalid escape code {}", ch),
                                 std::vector<std::string>());
      }

      base = i + 1;
      interp = false;
    } else {
      switch (ch) {
      case '%':
        commit_strtab(i);
        interp = true;
        break;
      case '(':
        commit_strtab(i);
        bytecode.push_back(Op::begin());
        base = i + 1;
        break;

      case ')':
        commit_strtab(i);
        bytecode.push_back(Op::end());
        base = i + 1;
        break;
      }
    }
  }

  commit_strtab(fmt.size());

  validate(this);
}

void Formatter::format(CmdOutput &output, int fd) {
  Output out(fd);

  std::array<TempFile *, 2> streams{output.out.get(), output.err.get()};

  bool ignore = false;
  size_t context{};

  for (size_t i{}; i < bytecode.size(); i++) {
    const Op &op = bytecode[i];

    if (ignore) {
      switch (op.code) {
      case Op::GroupBegin:
        context++;
        break;
      case Op::GroupEnd:
        context--;
        break;
      default:
        if (context == 0) {
          ignore = false;
        }
        continue;
      }

      if (context == 0) {
        ignore = false;
      }

      if (ignore)
        continue;
    }

    switch (op.code) {
    case Op::StrTab:
      out.push(strtab[op.arg]);
      break;

    case Op::Status:
      out.push(std::to_string(output.status));
      break;

    case Op::Cwd:
      switch (op.arg) {
      case 0:
        out.push(std::filesystem::relative(output.cwd, base_dir).string());
        break;
      case 1:
        out.push(output.cwd.string());
        break;
      }
      break;

    case Op::Emit:
      out.flush();
      streams[op.arg]->dump(fd);
      break;

    case Op::CheckSuccess:
      ignore = output.status != 0;
      break;

    case Op::CheckFailure:
      ignore = output.status == 0;
      break;

    case Op::CheckStream:
      ignore = streams[op.arg]->empty();
      break;

    default:
      break;
    }
  }

  out.flush();
}
