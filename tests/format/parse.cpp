
#include "format.h"

#include <gtest/gtest.h>

TEST(Formatter, parse_static_str) {
  Formatter formatter("hello");
  std::vector<std::string_view> strtab{"hello"};
  std::vector<Op> bytecode{Op::strtab(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_status) {
  Formatter formatter("%s");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::status()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_output) {
  Formatter formatter("%o");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::emit(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_error) {
  Formatter formatter("%e");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::emit(1)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_cwd) {
  Formatter formatter("%c");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::cwd()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_success_check) {
  Formatter formatter("%S");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_success()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_failure_check) {
  Formatter formatter("%F");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_failure()};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_stdout_check) {
  Formatter formatter("%O");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_stream(0)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_stderr_check) {
  Formatter formatter("%E");
  std::vector<std::string_view> strtab{};
  std::vector<Op> bytecode{Op::check_stream(1)};

  EXPECT_EQ(formatter.strtab, strtab);
  EXPECT_EQ(formatter.bytecode, bytecode);
}

TEST(Formatter, parse_invalid_code) {
  EXPECT_ANY_THROW({ Formatter formatter("%Z"); });
}
