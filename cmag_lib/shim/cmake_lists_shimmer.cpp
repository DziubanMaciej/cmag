#include "cmake_lists_shimmer.h"

#include "cmag_lib/shim/postamble.h"
#include "cmag_lib/utils/error.h"

#include <cstdio>
#include <fstream>
#include <string>

CMakeListsShimmer::CMakeListsShimmer(const fs::path &directory) : cmake_lists_path(directory / "CMakeLists.txt"),
                                                                  cmake_lists_backup_path(directory / "CMakeLists.txt.real") {}

CMakeListsShimmer::~CMakeListsShimmer() {
    if (isShimmed) {
        unshim();
    }
}

ShimResult CMakeListsShimmer::shim() {
    FATAL_ERROR_IF(isShimmed, "Already shimmed");

    if (!fs::is_regular_file(cmake_lists_path)) {
        printf("xd %s\n", cmake_lists_path.string().c_str());
        return ShimResult::InvalidDirectory;
    }

    std::error_code errorCode{};
    fs::rename(cmake_lists_path, cmake_lists_backup_path, errorCode);
    if (errorCode) {
        return ShimResult::NoPermission;
    }

    std::ifstream input{cmake_lists_backup_path};
    if (!input) {
        return ShimResult::NoPermission;
    }
    std::ofstream output{cmake_lists_path, std::ios::out};
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
    fs::rename(cmake_lists_backup_path, cmake_lists_path, errorCode);
    if (errorCode) {
        return ShimResult::NoPermission;
    }

    isShimmed = false;
    return ShimResult::Success;
}