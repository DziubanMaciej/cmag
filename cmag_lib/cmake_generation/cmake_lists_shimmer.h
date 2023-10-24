#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem; // TODO move to some shared header

enum class ShimResult {
    Success,
    InvalidDirectory,
    NoPermission,
};

class CMakeListsShimmer {
public:
    CMakeListsShimmer(const fs::path &directory);
    ~CMakeListsShimmer();

    ShimResult shim();
    ShimResult unshim();

private:
    const fs::path cmake_lists_path;
    const fs::path cmake_lists_backup_path;
    bool isShimmed = false;
};
