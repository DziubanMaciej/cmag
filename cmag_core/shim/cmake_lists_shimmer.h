#pragma once

#include "cmag_core/utils/filesystem.h"

#include <string>

enum class ShimResult {
    Success,
    InvalidDirectory,
    NoPermission,
};

class CMakeListsShimmer {
public:
    CMakeListsShimmer(const fs::path &sourceDirectory);
    ~CMakeListsShimmer();

    ShimResult shim();
    ShimResult unshim();

private:
    void generateBackupPath();

    const fs::path sourceDirectory;
    const fs::path cmakeListsPath;
    fs::path cmakeListsBackupPath = {};
    bool isShimmed = false;
};
