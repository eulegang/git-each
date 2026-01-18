
#include "cli.h"

#include <gtest/gtest.h>
#include <string_view>
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
