#pragma once

#include <filesystem>
namespace fs = std::filesystem;

fs::path getExeLocation();
bool openHyperlink(const char *path);
