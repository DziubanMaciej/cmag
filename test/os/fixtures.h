#pragma once

#include "cmag_lib/utils/filesystem.h"

#include <gtest/gtest.h>

const static inline fs::path srcProjectsRoot = fs::path{SRC_PROJECTS_ROOT};
const static inline fs::path dstProjectsRoot = fs::path{DST_PROJECTS_ROOT};

struct CmagOsTest : ::testing::Test {
    // TODO rename ProjectInfo -> TestWorkspace
    struct ProjectInfo {
        bool valid = false;
        fs::path sourcePath;
        fs::path buildPath;
        fs::path graphvizPath;
    };

    void TearDown() override {
        for (auto &project : projects) {
            removeFile(project.sourcePath);
        }
    }

    ProjectInfo prepareProject(std::string_view name) {
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

        // Return project description
        ProjectInfo project = {};
        project.valid = !::testing::Test::HasFailure();
        project.sourcePath = dstProjectDir;
        project.buildPath = dstProjectDir / "build";
        project.graphvizPath = dstProjectDir / "build" / "a.dot";
        projects.push_back(project);
        return project;
    }

    static void createDir(const fs::path &dir) {
        std::error_code err{};
        fs::create_directory(dir, err);
        EXPECT_EQ(0u, err.value());
    }

    static void copyDir(const fs::path &src, const fs::path &dst) {
        std::error_code err{};
        fs::copy(src, dst, fs::copy_options::recursive, err);
        EXPECT_EQ(0u, err.value());
    }

    static void removeFile(const fs::path &dir) {
        if (fs::exists(dir)) {
            std::error_code err{};
            fs::remove_all(dir, err);
            EXPECT_EQ(0, err.value());
        }
        EXPECT_FALSE(fs::exists(dir));
    }

    static void clearPermissions(const fs::path &file) {
        std::error_code err{};
        fs::permissions(file, fs::perms::none, err);
        EXPECT_EQ(0, err.value());
    }

private:
    std::vector<ProjectInfo> projects = {};
};

struct RaiiStdoutCapture {
    RaiiStdoutCapture() {
        testing::internal::CaptureStdout();
    }

    ~RaiiStdoutCapture() {
        testing::internal::GetCapturedStdout();
    }
};