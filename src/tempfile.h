#ifndef _GIT_EACH_TEMPFILE_H
#define _GIT_EACH_TEMPFILE_H

#include <atomic>
#include <cstring>
#include <exception>
#include <filesystem>
#include <format>
#include <string_view>

class TempFileException : public std::exception {
  std::string message;

public:
  TempFileException(const std::filesystem::path &path, std::string_view context,
                    int err) {
    std::string err_msg(strerror(err));
    message = std::format("{} {} {}", err_msg, path.string(), context);
  }
  const char *what() const noexcept override { return message.c_str(); }
};

class TempFile final {
  int _fd;
  std::filesystem::path _path;

public:
  TempFile(std::filesystem::path path);
  ~TempFile();

  void dup(int fd) const;
  void dump(int fd) const;
  const std::filesystem::path &path() const;
  int fd() const;
  bool empty() const;
};

class TempInst final {
  std::uint64_t id;
  std::shared_ptr<std::filesystem::path> path;

public:
  TempInst(std::shared_ptr<std::filesystem::path> path, std::uint64_t id);

  std::unique_ptr<TempFile> tmp(std::string_view);
};

class TempBase final {
  std::shared_ptr<std::filesystem::path> _path;
  std::atomic<std::uint64_t> _next;

public:
  TempBase(std::string_view);
  ~TempBase();
  TempInst inst();
};

#endif
