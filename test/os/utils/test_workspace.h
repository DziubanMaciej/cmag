#pragma once

#include "test/os/utils/file_operations.h"

struct TestWorkspace {
    const static inline fs::path srcProjectsRoot = fs::path{SRC_PROJECTS_ROOT};
    const static inline fs::path dstProjectsRoot = fs::path{DST_PROJECTS_ROOT};

    bool valid = false;
    fs::path sourcePath;
    fs::path buildPath;
    fs::path graphvizPath;

    ~TestWorkspace() {
        removeFile(sourcePath);
    }

    TestWorkspace(TestWorkspace &&) = default;
    TestWorkspace &operator=(TestWorkspace &&) = default;

    static TestWorkspace prepare(std::string_view name) {
        const fs::path srcProjectDir = srcProjectsRoot / name;
        const fs::path dstProjectDir = dstProjectsRoot / name;

        // Cleanup src dir.
        EXPECT_TRUE(fs::is_regular_file(srcProjectDir / "CMakeLists.txt"));
        EXPECT_TRUE(fs::is_directory(srcProjectDir));
        removeFile(srcProjectDir / "build");

        // Cleanup dst dir.
        removeFile(dstProjectsRoot);
        createDir(dstProjectsRoot);
        createDir(dstProjectDir);

        // Copy src dir to dst dir.
        copyDir(srcProjectDir, dstProjectDir);
        createDir(dstProjectDir / "build");

        // Return workspace description
        TestWorkspace workspace = {};
        workspace.valid = !::testing::Test::HasFailure();
        workspace.sourcePath = dstProjectDir;
        workspace.buildPath = dstProjectDir / "build";
        workspace.graphvizPath = dstProjectDir / "build" / "a.dot";
        return workspace;
    }
};
