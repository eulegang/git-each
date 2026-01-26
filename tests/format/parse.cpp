
#include "format.h"

#include <gtest/gtest.h>

const std::filesystem::path dir = "/home/xyz/proj";

TEST(Formatter, parse_static_str) {
  Formatter formatter("hello", dir);
  std::vector<std::string_view> strtab{"hello"};
  std::vector<Op> bytecode{Op::strtab(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_status) {
  Formatter formatter("%s", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::status()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_output) {
  Formatter formatter("%o", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::emit(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_error) {
  Formatter formatter("%e", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::emit(1)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_cwd) {
  Formatter formatter("%c", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::cwd(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_cwd_abs) {
  Formatter formatter("%C", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::cwd(1)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_success_check) {
  Formatter formatter("%S", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_success()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_failure_check) {
  Formatter formatter("%F", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_failure()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_stdout_check) {
  Formatter formatter("%O", dir);
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_stream(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_stderr_check) {
  Formatter formatter("%S(Success)", dir);
  std::vector<std::string_view> strtab{"Success"};
  std::vector<Op> bytecode{Op::check_success(), Op::begin(), Op::strtab(0),
                           Op::end()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_invalid_code) {
  EXPECT_ANY_THROW({ Formatter formatter("%Z", dir); });
}

TEST(Formatter, parse_unbalanced) {
  EXPECT_ANY_THROW({ Formatter formatter("%s(", dir); });
}

TEST(Formatter, parse_underflow_group) {
  EXPECT_ANY_THROW({ Formatter formatter("%s)(", dir); });
}

TEST(Formatter, parse_literal_parens) {
  Formatter formatter("%(%)", dir);
  std::vector<std::string_view> strtab{"(", ")"};
  std::vector<Op> bytecode{Op::strtab(0), Op::strtab(1)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}
