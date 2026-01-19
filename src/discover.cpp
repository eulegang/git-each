#include <filesystem>
#include <functional>
#include <queue>

using path_t = std::filesystem::path;

bool is_repo(path_t path);
void push_dirs(path_t path, std::queue<path_t> &fringe);

void discover(path_t path, std::function<void(std::filesystem::path)> action) {
  std::queue<path_t> fringe;
  fringe.push(path);

  for (; !fringe.empty(); fringe.pop()) {
    path_t cur = fringe.front();

    if (!std::filesystem::is_directory(cur)) {
      continue;
    }

    if (is_repo(cur)) {
      action(cur);
    } else {
      push_dirs(cur, fringe);
    }
  }
}

bool is_repo(path_t path) {
  for (const auto &ent : std::filesystem::directory_iterator(path)) {
    if (!ent.is_directory()) {
      continue;
    }

    if (ent.path().filename() == ".git")
      return true;
  }

  return false;
}

void push_dirs(path_t path, std::queue<path_t> &fringe) {
  for (const auto &ent : std::filesystem::directory_iterator(path)) {
    if (ent.is_directory()) {
      fringe.push(ent);
    }
  }
}
