
#include "cli.h"

#include <gtest/gtest.h>
#include <string_view>
#include <thread>
#include <vector>

TEST(CLI, parse_skip_binname) {
  Cli cli{
      "git-each",
      "status",
  };

  std::vector<std::string_view> expected{"status"};

  EXPECT_EQ(cli.args, expected);
}

TEST(CLI, parse_help) {
  Cli cli{
      "git-each",
      "--help",
  };

  EXPECT_TRUE(cli.help);

  Cli shrt{
      "git-each",
      "-h",
  };

  EXPECT_TRUE(shrt.help);
}

TEST(CLI, parse_version) {
  Cli cli{
      "git-each",
      "--version",
  };

  EXPECT_TRUE(cli.version);

  Cli shrt{
      "git-each",
      "-V",
  };

  EXPECT_TRUE(shrt.version);
}

TEST(CLI, parse_fuse) {
  Cli cli{
      "git-each",
      "--",
      "--version",

  };

  EXPECT_FALSE(cli.version);
}

TEST(CLI, parse_dir) {
  Cli cli{
      "git-each",
      "-d",
      "./src",

  };

  EXPECT_EQ(cli.dir, "./src");
}

TEST(CLI, parse_system) {
  Cli cli{
      "git-each",
      "--system",
  };

  EXPECT_TRUE(cli.system);
  Cli shrt{
      "git-each",
      "-s",
  };

  EXPECT_TRUE(shrt.system);
}

TEST(CLI, parse_discover) {
  Cli cli{
      "git-each",
      "--discover",
  };

  EXPECT_TRUE(cli.discover);
  Cli shrt{
      "git-each",
      "-D",
  };

  EXPECT_TRUE(shrt.discover);
}

TEST(CLI, parse_jobs_undef) {
  Cli cli{
      "git-each",
  };

  EXPECT_EQ(cli.jobs, (int)std::thread::hardware_concurrency());
}

TEST(CLI, parse_jobs_specified) {
  Cli cli{"git-each", "-j", "4"};

  EXPECT_EQ(cli.jobs, 4);
}

TEST(CLI, empty_flags) {
  Cli cli{
      "git-each",
  };

  EXPECT_FALSE(cli.help);
  EXPECT_FALSE(cli.version);
  EXPECT_FALSE(cli.system);
  EXPECT_FALSE(cli.discover);
}

TEST(CLI, parse_combo) {
  Cli cli{
      "git-each", "-f", "%s\n%o", "--", "status", "-s",
  };

  EXPECT_EQ(cli.format, "%s\n%o");

  std::vector<std::string_view> expected{
      "status",
      "-s",
  };

  EXPECT_EQ(cli.args, expected);
}
