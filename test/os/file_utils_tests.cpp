#include "cmag_core/utils/file_utils.h"
#include "test/os/fixtures.h"

#include <fstream>

struct FileUtilsTest : CmagOsTest {
    static void verifyDebugFileContent(const std::string &content, size_t size) {
        ASSERT_EQ(size, content.size());
        EXPECT_EQ('{', content.front());
        EXPECT_EQ('}', content.back());
    }

    static void createDebugFile(fs::path path, size_t size) {
        ASSERT_NE(0, size);

        const size_t chunkSize = 4096;
        char chunk[4096];
        std::fill(chunk, chunk + chunkSize, '.');

        std::ofstream file{path, std::ios::out};
        file << '{';
        for (size_t sizeLeft = size - 2; sizeLeft > 0;) {
            size_t currentSize = std::min(sizeLeft, chunkSize);
            file.write(chunk, currentSize);

            sizeLeft -= currentSize;
        }
        file << '}';
    }
};

TEST_F(FileUtilsTest, givenSmallFileThenReadItProperly) {
    TestWorkspace workspace = TestWorkspace::prepareEmpty();
    ASSERT_TRUE(workspace.valid);
    const fs::path file = workspace.sourcePath / "file.txt";

    const size_t size = 10;
    createDebugFile(file, size);

    std::optional<std::string> content = ::readFile(file);
    ASSERT_TRUE(content.has_value());

    verifyDebugFileContent(content.value(), size);
}

TEST_F(FileUtilsTest, givenBigFileThenReadItProperly) {
    TestWorkspace workspace = TestWorkspace::prepareEmpty();
    ASSERT_TRUE(workspace.valid);
    const fs::path file = workspace.sourcePath / "file.txt";

    const size_t size = 16 * 1024 * 1024;
    createDebugFile(file, size);

    std::optional<std::string> content = ::readFile(file);
    ASSERT_TRUE(content.has_value());

    verifyDebugFileContent(content.value(), size);
}

TEST_F(FileUtilsTest, givenNoFileThenReturnEmptyOptional) {
    TestWorkspace workspace = TestWorkspace::prepareEmpty();
    ASSERT_TRUE(workspace.valid);
    const fs::path file = workspace.sourcePath / "file.txt";

    std::optional<std::string> content = ::readFile(file);
    EXPECT_FALSE(content.has_value());
}
