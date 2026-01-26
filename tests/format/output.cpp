
#include "cmd.h"
#include "format.h"
#include <gtest/gtest.h>
#include <memory>
#include <unistd.h>

TempBase base("format-tests");
const std::filesystem::path dir = "/home/xyz/proj";

std::shared_ptr<CmdOutput> make_output() {
  TempInst inst = base.inst();
  std::unique_ptr<TempFile> out = inst.tmp("stdout");
  std::unique_ptr<TempFile> err = inst.tmp("stderr");
  std::filesystem::path cwd("/home/xyz/proj/thingy");

  return std::make_shared<CmdOutput>(0, cwd, std::move(out), std::move(err));
}

std::string run_formatter(Formatter &formatter,
                          std::shared_ptr<CmdOutput> cmd) {
  int fds[2];
  if (pipe(fds) == -1) {
    throw "failed to build pipe";
  }

  formatter.format(*cmd, fds[1]);
  close(fds[1]);

  std::string s;

  char buf[4096];
  ssize_t r = 0;

  while ((r = read(fds[0], buf, 4096)) > 0) {
    s.append(std::string_view(buf, r));
  }

  return s;
}

TEST(Formmatter, output_static) {
  auto output = make_output();
  Formatter formatter("static text", dir);

  auto content = run_formatter(formatter, output);
  EXPECT_EQ(content, "static text");
}

TEST(Formatter, output_status) {
  auto output = make_output();
  Formatter formatter("%s", dir);
  {
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "0");
  }

  output->status = 32;
  {
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "32");
  }
}
TEST(Formatter, output_cwd) {
  auto output = make_output();
  Formatter formatter("%c", dir);
  auto content = run_formatter(formatter, output);
  EXPECT_EQ(content, "thingy");
}

TEST(Formatter, output_cwd_abs) {
  auto output = make_output();
  Formatter formatter("%C", dir);
  auto content = run_formatter(formatter, output);
  EXPECT_EQ(content, "/home/xyz/proj/thingy");
}

TEST(Formatter, output_output) {
  auto output = make_output();
  write(output->out->fd(), "hello", 5);
  {
    Formatter formatter("%o", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "hello");
  }

  {
    Formatter formatter("%e", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "");
  }
}

TEST(Formatter, output_error) {
  auto output = make_output();
  write(output->err->fd(), "hello", 5);
  {
    Formatter formatter("%o", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "");
  }

  {
    Formatter formatter("%e", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "hello");
  }
}

TEST(Formatter, output_status_check) {
  auto output = make_output();
  output->status = 0;

  {
    Formatter formatter("%S.", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, ".");
  }

  {
    Formatter formatter("%F!", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "");
  }

  output->status = 32;

  {
    Formatter formatter("%S.", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "");
  }

  {
    Formatter formatter("%F!", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "!");
  }
}

TEST(Formatter, output_grouped) {
  auto output = make_output();
  output->status = 0;

  {
    Formatter formatter("%S(Succeeded) - %s", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "Succeeded - 0");
  }

  {
    Formatter formatter("%F(Failed) - %s", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, " - 0");
  }

  output->status = 32;

  {
    Formatter formatter("%S(Succeeded) - %s", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, " - 32");
  }

  {
    Formatter formatter("%F(Failed) - %s", dir);
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "Failed - 32");
  }
}

TEST(Formatter, output_grouped_nested) {
  auto output = make_output();
  output->status = 0;
  Formatter formatter("%S(Succeeded %s%O( xyz))", dir);

  {
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "Succeeded 0");
  }

  write(output->out->fd(), "hello", 5);

  {
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "Succeeded 0 xyz");
  }

  output->status = 32;
  {
    auto content = run_formatter(formatter, output);
    EXPECT_EQ(content, "");
  }
}
