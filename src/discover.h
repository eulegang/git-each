#ifndef _GIT_EACH_DISCOVER_H
#define _GIT_EACH_DISCOVER_H

#include <filesystem>
#include <functional>

void discover(std::filesystem::path path,
              std::function<void(std::filesystem::path)>);

#endif
