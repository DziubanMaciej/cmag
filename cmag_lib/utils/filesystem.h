#pragma once

#include <filesystem>
namespace fs = std::filesystem;

fs::path getExeLocation();
void openHyperlink(const char *path);
