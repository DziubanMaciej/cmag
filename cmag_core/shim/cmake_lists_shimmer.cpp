#include "cmake_lists_shimmer.h"

#include "cmag_core/utils/error.h"
#include "generated/postamble.h"

#include <cstdio>
#include <fstream>
#include <string>

CMakeListsShimmer::CMakeListsShimmer(const fs::path &sourceDirectory) : sourceDirectory(sourceDirectory),
                                                                        cmakeListsPath(sourceDirectory / "CMakeLists.txt") {}

CMakeListsShimmer::~CMakeListsShimmer() {
    if (isShimmed) {
        // TODO print warning?
        unshim();
    }
}

ShimResult CMakeListsShimmer::shim() {
    FATAL_ERROR_IF(isShimmed, "Already shimmed");

    if (!fs::is_regular_file(cmakeListsPath)) {
        return ShimResult::InvalidDirectory;
    }

    generateBackupPath();
    std::error_code errorCode{};
    fs::rename(cmakeListsPath, cmakeListsBackupPath, errorCode);
    if (errorCode) {
        return ShimResult::NoPermission;
    }

    std::ifstream input{cmakeListsBackupPath};
    if (!input) {
        return ShimResult::NoPermission;
    }
    std::ofstream output{cmakeListsPath, std::ios::out};
    if (!output) {
        return ShimResult::NoPermission;
    }
    for (std::string line{}; input;) {
        std::getline(input, line);
        output << line << "\n";
    }
    output << postamble << "\n";

    isShimmed = true;
    return ShimResult::Success;
}

ShimResult CMakeListsShimmer::unshim() {
    FATAL_ERROR_IF(!isShimmed, "Not shimmed");

    std::error_code errorCode{};
    fs::rename(cmakeListsBackupPath, cmakeListsPath, errorCode);
    if (errorCode) {
        return ShimResult::NoPermission;
    }

    isShimmed = false;
    return ShimResult::Success;
}

void CMakeListsShimmer::generateBackupPath() {
    // Should we put some boundary on that loop?
    for (size_t i = 0; true; i++) {
        std::string dirName = std::string{"CMakeLists.txt.backup"} + std::to_string(i) + ".cmake";
        cmakeListsBackupPath = sourceDirectory / dirName;
        if (!fs::exists(cmakeListsBackupPath)) {
            return;
        }
    }
}
