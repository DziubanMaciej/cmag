#include "cmag_core/browser/browser_argument_parser.h"

#include <gtest/gtest.h>

TEST(BrowserArgumentParserTest, givenEmptyArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag_browser"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
    EXPECT_FALSE(parser.getErrorMessage().empty());
}

TEST(BrowserArgumentParserTest, givenOnlyFileSpecifiedThenItIsValid) {
    const char *argv[] = {"cmag_browser", "my_file"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("my_file", parser.getProjectFilePath().c_str());
}

TEST(BrowserArgumentParserTest, givenMultipleFilesSpecifiedThenItIsInvalid) {
    const char *argv[] = {"cmag_browser", "my_file", "my_other_file"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
    EXPECT_FALSE(parser.getErrorMessage().empty());
}

TEST(BrowserArgumentParserTest, givenVersionArgumentSpecifiedThenItIsProcessed) {
    {
        const char *argv[] = {"cmag_browser", "my_file"};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        BrowserArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_FALSE(parser.getShowVersion());
    }
    {
        const char *argv[] = {"cmag_browser", "-v", "my_file"};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        BrowserArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_TRUE(parser.getShowVersion());
    }
    {
        const char *argv[] = {"cmag_browser", "my_file", "-v"};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        BrowserArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_TRUE(parser.getShowVersion());
    }
}
