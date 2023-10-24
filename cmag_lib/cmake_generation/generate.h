#pragma once

#include "cmag_lib/cmake_generation/argument_parser.h"
#include "cmag_lib/cmake_generation/cmake_lists_shimmer.h"
#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/filesystem.h"
#include "cmag_lib/utils/subprocess.h"

#include <vector>

int generateCmake(ArgumentParser &args) {
    // Shim original CMakeLists.txt and insert extra CMake code to query information about the build-system
    // and save it to a file.
    CMakeListsShimmer shimmer{args.getSourcePath()};
    const ShimResult shimResult = shimmer.shim();
    switch (shimResult) {
    case ShimResult::Success:
        break;
    case ShimResult::InvalidDirectory:
        LOG_ERROR("failed CMakeLists.txt shimming (invalid source directory).\n");
        return 1;
    case ShimResult::NoPermission:
        LOG_ERROR("failed CMakeLists.txt shimming (no permission).\n");
        return 1;
    default:
        UNREACHABLE_CODE;
    }

    // Call CMake
    std::vector<const char *> cmakeArgs = args.constructArgsForCmake();
    const SubprocessResult result = runSubprocess(static_cast<int>(cmakeArgs.size()), cmakeArgs.data());
    switch (result) {
    case SubprocessResult::Success:
        return 0;
        break;
    case SubprocessResult::CreationFailed:
        LOG_ERROR("running CMake failed.");
        return 1;
    case SubprocessResult::ProcessKilled:
        LOG_ERROR("CMake has been killed.");
        return 1;
    case SubprocessResult::ProcessFailed:
        LOG_ERROR("CMake failed.");
        return 1;
    default:
        UNREACHABLE_CODE;
    }
}
