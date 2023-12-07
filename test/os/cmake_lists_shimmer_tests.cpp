#include "cmag_core/shim/cmake_lists_shimmer.h"
#include "cmag_core/utils/file_utils.h"
#include "test/os/fixtures.h"

struct CMakeListsShimmerTests : CmagOsTest {
    void SetUp() override {
        CmagOsTest::SetUp();

        workspace = TestWorkspace::prepare("simple");
        ASSERT_TRUE(workspace.valid);
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

    TestWorkspace workspace = {};
};

TEST_F(CMakeListsShimmerTests, givenCMakeListsFileThenCorrectlyShimAndUnshim) {
    const fs::path backupFile = workspace.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{workspace.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    EXPECT_EQ(ShimResult::Success, shimmer.unshim());
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, givenNoCmakeListsFileWhenShimmngThenReturnError) {
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";
    removeFile(cmakeLists);

    CMakeListsShimmer shimmer{workspace.sourcePath};
    EXPECT_EQ(ShimResult::InvalidDirectory, shimmer.shim());
}

TEST_F(CMakeListsShimmerTests, givenCMakeListsIsDirectoryWhenShimmngThenReturnError) {
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";
    removeFile(cmakeLists);
    createDir(cmakeLists);

    CMakeListsShimmer shimmer{workspace.sourcePath};
    EXPECT_EQ(ShimResult::InvalidDirectory, shimmer.shim());
}

TEST_F(CMakeListsShimmerTests, givenNoPermissionsToCMakeListsWhenShimmngThenReturnError) {
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";
    ASSERT_TRUE(clearPermissions(cmakeLists));

    CMakeListsShimmer shimmer{workspace.sourcePath};
    EXPECT_EQ(ShimResult::NoPermission, shimmer.shim());

    ASSERT_TRUE(restorePermissions(cmakeLists));
}

TEST_F(CMakeListsShimmerTests, givenAlreadyShimmedWhenShimmngThenUseDifferentName) {
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";
    const fs::path backupFile0 = workspace.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path backupFile1 = workspace.sourcePath / "CMakeLists.txt.backup1.cmake";

    CMakeListsShimmer shimmer0{workspace.sourcePath};
    EXPECT_EQ(ShimResult::Success, shimmer0.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile0));

    CMakeListsShimmer shimmer1{workspace.sourcePath};
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
    const fs::path backupFile = workspace.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{workspace.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    removeFile(backupFile);

    EXPECT_EQ(ShimResult::NoPermission, shimmer.unshim());
    EXPECT_TRUE(isShimmedFile(cmakeLists)); // leave shimmed file, so we can recover
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, givenShimFileDeletedWhenUnshimmingThenIgnore) {
    const fs::path backupFile = workspace.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";

    CMakeListsShimmer shimmer{workspace.sourcePath};

    EXPECT_EQ(ShimResult::Success, shimmer.shim());
    EXPECT_TRUE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(isShimmedFile(backupFile));

    removeFile(cmakeLists);

    EXPECT_EQ(ShimResult::Success, shimmer.unshim());
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}

TEST_F(CMakeListsShimmerTests, whenDestructorCalledThenCallUnshim) {
    const fs::path backupFile = workspace.sourcePath / "CMakeLists.txt.backup0.cmake";
    const fs::path cmakeLists = workspace.sourcePath / "CMakeLists.txt";

    {
        CMakeListsShimmer shimmer{workspace.sourcePath};

        EXPECT_EQ(ShimResult::Success, shimmer.shim());
        EXPECT_TRUE(isShimmedFile(cmakeLists));
        EXPECT_FALSE(isShimmedFile(backupFile));
    }
    EXPECT_FALSE(isShimmedFile(cmakeLists));
    EXPECT_FALSE(fs::exists(backupFile));
}
