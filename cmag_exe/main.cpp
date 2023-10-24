#include "cmag_lib/cmake_generation/argument_parser.h"
#include "cmag_lib/cmake_generation/cmake_lists_shimmer.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/subprocess.h"

#include <cstdio>
#include <iostream>

int main(int argc, const char **argv) {
    ArgumentParser argParser{argc, argv};

    const fs::path &sourcePath = argParser.getSourcePath();
    if (sourcePath.empty()) {
        printf("ERROR: empty source dir.\n");
        return 1;
    }
    if (!fs::is_directory(sourcePath)) {
        printf("ERROR: not a valid source dir.\n");
        return 1;
    }

    CMakeListsShimmer shimmer{sourcePath};
    switch (shimmer.shim()) {
    case ShimResult::Success:
        break;
    case ShimResult::InvalidDirectory:
        printf("ERROR: failed CMakeLists.txt shimming (invalid source directory).\n");
        break;
    case ShimResult::NoPermission:
        printf("ERROR: failed CMakeLists.txt shimming (no permission).\n");
        break;
    default:
        FATAL_ERROR("Invalid shim error");
    }

    std::vector<const char *> cmakeArguments = {};
    cmakeArguments.reserve(argc - 1 + argParser.getExtraArgs().size());
    for (int i = 1; i < argc; i++) {
        cmakeArguments.push_back(argv[i]);
    }
    for (const auto &extraArg : argParser.getExtraArgs()) {
        cmakeArguments.push_back(extraArg.c_str());
    }
    cmakeArguments.push_back(nullptr);
    SubprocessResult result = runSubprocess(static_cast<int>(cmakeArguments.size()), cmakeArguments.data());
    switch (result) {
    case SubprocessResult::Success:
        break;
    case SubprocessResult::CreationFailed:
        printf("ERROR: running CMake failed.\n");
        return 1;
    case SubprocessResult::ProcessKilled:
        printf("ERROR: CMake has been killed.\n");
        return 1;
    case SubprocessResult::ProcessFailed:
        printf("ERROR: CMake failed.\n");
        return 1;
    default:
        FATAL_ERROR("Invalid subprocess result");
    }
}