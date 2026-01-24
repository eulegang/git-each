#include "format.h"
#include "cmd.h"

bool operator==(const Op a, const Op b) {
  return a.code == b.code && a.arg == b.arg;
}
Formatter::Formatter(std::string_view fmt) : strtab{}, bytecode{} {
  size_t base = 0;
  size_t size = 0;
  bool interp = false;

  const auto commit_strtab = [this, fmt, &base, &size]() {
    if (size != 0) {
      const size_t index = this->strtab.size();
      this->strtab.push_back(fmt.substr(base, size));
      this->bytecode.push_back(Op::strtab(index));

      base += size;
    }
  };

  for (const auto ch : fmt) {
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
        bytecode.push_back(Op::cwd());
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

      default:
        throw "invalid operation code";
      }

      interp = false;
    } else if (ch == '%') {
      commit_strtab();
      interp = true;
    } else {
      size++;
    }
  }

  commit_strtab();
}

void Formatter::format(CmdOutput &output) { (void)output; }
