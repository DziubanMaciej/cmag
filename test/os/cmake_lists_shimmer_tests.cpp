#include "cmag_lib/shim/cmake_lists_shimmer.h"
#include "cmag_lib/utils/file_utils.h"
#include "test/os/fixtures.h"

struct CMakeListsShimmerTests : CmagOsTest {
    void SetUp() override {
        CmagOsTest::SetUp();

        project = prepareProject("simple");
        ASSERT_TRUE(project.valid);
    }

    static bool isShimmedFile(const fs::path &file) {
        std::optional<std::string> contents = readFile(file);
        if (!contents.has_value()) {
            EXPECT_TRUE(contents.has_value());
            return false;
        }

        return contents.value().find("CMAG POSTAMBLE BEGIN") != std::string::npos &&
               contents.value().rfind("CMAG POSTAMBLE END") != std::string::npos;
    }

    ProjectInfo project = {};
};

TEST_F(CMakeListsShimmerTests, givenCMakeListsFileThenCorrectlyShimAndUnshim) {
    const fs::path backupFile = project.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{project.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    EXPECT_EQ(ShimResult::Success, shimmer.unshim());
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, givenNoCmakeListsFileWhenShimmngThenReturnError) {
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";
    removeFile(cmakeLists);

    CMakeListsShimmer shimmer{project.sourcePath};
    EXPECT_EQ(ShimResult::InvalidDirectory, shimmer.shim());
}

TEST_F(CMakeListsShimmerTests, givenCMakeListsIsDirectoryWhenShimmngThenReturnError) {
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";
    removeFile(cmakeLists);
    createDir(cmakeLists);

    CMakeListsShimmer shimmer{project.sourcePath};
    EXPECT_EQ(ShimResult::InvalidDirectory, shimmer.shim());
}

TEST_F(CMakeListsShimmerTests, givenNoPermissionsToCMakeListsWhenShimmngThenReturnError) {
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";
    clearPermissions(cmakeLists);

    CMakeListsShimmer shimmer{project.sourcePath};
    EXPECT_EQ(ShimResult::NoPermission, shimmer.shim());
}

TEST_F(CMakeListsShimmerTests, givenAlreadyShimmedWhenShimmngThenUseDifferentName) {
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";
    const fs::path backupFile0 = project.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path backupFile1 = project.sourcePath / "CMakeLists.txt.backup1.cmake";

    CMakeListsShimmer shimmer0{project.sourcePath};
    EXPECT_EQ(ShimResult::Success, shimmer0.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile0));

    CMakeListsShimmer shimmer1{project.sourcePath};
    EXPECT_EQ(ShimResult::Success, shimmer1.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile0));
    EXPECT_TRUE(isShimmedFile(backupFile1));

    EXPECT_EQ(ShimResult::Success, shimmer1.unshim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile0));
    EXPECT_FALSE(fs::exists(backupFile1));

    EXPECT_EQ(ShimResult::Success, shimmer0.unshim());
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile0));
    EXPECT_FALSE(fs::exists(backupFile1));
}

TEST_F(CMakeListsShimmerTests, givenRealFileDeletedWhenUnshimmingThenReturnError) {
    const fs::path backupFile = project.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{project.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    removeFile(backupFile);

    EXPECT_EQ(ShimResult::NoPermission, shimmer.unshim());
    EXPECT_TRUE(isShimmedFile(cmakeLists)); // leave shimmed file, so we can recover
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, givenShimFileDeletedWhenUnshimmingThenIgnore) {
    const fs::path backupFile = project.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{project.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    removeFile(cmakeLists);

    EXPECT_EQ(ShimResult::Success, shimmer.unshim());
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, whenDestructorCalledThenCallUnshim) {
    const fs::path backupFile = project.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = project.sourcePath / "CMakeLists.txt";

    {
        CMakeListsShimmer shimmer{project.sourcePath};

        EXPECT_EQ(ShimResult::Success, shimmer.shim());
        EXPECT_TRUE(isShimmedFile(cmakeLists));
        EXPECT_FALSE(isShimmedFile(backupFile));
    }
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}
