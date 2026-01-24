#include "format.h"
#include "cmd.h"

bool operator==(const Op a, const Op b) {
  return a.code == b.code && a.arg == b.arg;
}

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
    case Op::CheckSuccess:
    case Op::CheckFailure:
    case Op::Cwd:
    case Op::Status:
      break;
    }
  }

  if (group_count != 0) {
    msg.push_back("unbalanced group");
  }

  if (msg.size() > 0) {
    throw msg;
  }
}

Formatter::Formatter(std::string_view fmt) : strtab{}, bytecode{} {
  size_t base = 0;
  bool interp = false;

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

void Formatter::format(CmdOutput &output) { (void)output; }
