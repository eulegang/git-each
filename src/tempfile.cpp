
#include "tempfile.h"

#include <filesystem>
#include <memory>

#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <unistd.h>

#include <format>

using path_t = std::filesystem::path;

TempBase::TempBase(std::string_view key) {
  pid_t pid = getpid();
  path_t path = std::filesystem::temp_directory_path();

  _path = std::make_shared<path_t>(path / key / std::to_string(pid));

  std::filesystem::create_directories(*_path);
}

TempBase::~TempBase() {
  std::filesystem::remove_all(*_path);

  // try to remove base, but don't care if others
  try {
    std::filesystem::remove(_path->parent_path());
  } catch (...) {
  }
}

TempInst::TempInst(std::shared_ptr<std::filesystem::path> path,
                   std::uint64_t id)
    : id{id}, path{path} {

  std::filesystem::create_directories(*path / std::to_string(id));
}

TempFile::TempFile(path_t path) : _path{path} {
  _fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (_fd == -1) {
    throw TempFileException(path, "open", errno);
  }
}

TempFile::~TempFile() {
  std::filesystem::remove(_path);
  close(_fd);
}

TempInst TempBase::inst() {
  uint64_t id = _next.fetch_add(1);
  TempInst inst(_path, id);
  return inst;
}

const path_t &TempFile::path() const { return _path; }

int TempFile::fd() const { return _fd; }

std::unique_ptr<TempFile> TempInst::tmp(std::string_view name) {
  return std::make_unique<TempFile>(*path / std::to_string(id) / name);
}

void TempFile::dup(int fd) const {
  int res = dup2(_fd, fd);

  if (res == -1) {
    throw TempFileException(_path, "dup", errno);
  }
}

void TempFile::dump(int fd) const {
  ssize_t res;
  const off_t size = lseek(_fd, 0, SEEK_END);
  off_t offset = 0;

  if (size == -1) {
    throw TempFileException(_path, "lseek", errno);
  }

  res = sendfile(fd, _fd, &offset, size);

  if (res == -1) {
    throw TempFileException(_path, "lseek", errno);
  }
}

bool TempFile::empty() const {
  const off_t size = lseek(_fd, 0, SEEK_END);

  if (size == -1) {
    throw TempFileException(_path, "lseek", errno);
  }

  return size == 0;
}
