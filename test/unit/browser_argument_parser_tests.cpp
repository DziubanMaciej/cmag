#include "cmag_core/browser/browser_argument_parser.h"

#include <gtest/gtest.h>

struct BrowserArgumentParserTest : ::testing::Test {
    using ArgParserBooleanGetter = bool (*)(const BrowserArgumentParser &parser);
    static void testBooleanArg(const char *option, ArgParserBooleanGetter getter) {
        {
            const char *argv[] = {"cmag_browser", "my_file"};
            const int argc = sizeof(argv) / sizeof(argv[0]);
            BrowserArgumentParser parser{argc, argv};
            EXPECT_TRUE(parser.isValid());
            EXPECT_FALSE(getter(parser));
        }
        {
            const char *argv[] = {"cmag_browser", option, "my_file"};
            const int argc = sizeof(argv) / sizeof(argv[0]);
            BrowserArgumentParser parser{argc, argv};
            EXPECT_TRUE(parser.isValid());
            EXPECT_TRUE(getter(parser));
        }
        {
            const char *argv[] = {"cmag_browser", "my_file", option};
            const int argc = sizeof(argv) / sizeof(argv[0]);
            BrowserArgumentParser parser{argc, argv};
            EXPECT_TRUE(parser.isValid());
            EXPECT_TRUE(getter(parser));
        }
        {
            const char *argv[] = {"cmag_browser", "my_file", option, option};
            const int argc = sizeof(argv) / sizeof(argv[0]);
            BrowserArgumentParser parser{argc, argv};
            EXPECT_TRUE(parser.isValid());
            EXPECT_TRUE(getter(parser));
        }
    }
};

TEST_F(BrowserArgumentParserTest, givenEmptyArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag_browser"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
    EXPECT_FALSE(parser.getErrorMessage().empty());
}

TEST_F(BrowserArgumentParserTest, givenOnlyFileSpecifiedThenItIsValid) {
    const char *argv[] = {"cmag_browser", "my_file"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("my_file", parser.getProjectFilePath().c_str());
}

TEST_F(BrowserArgumentParserTest, givenMultipleFilesSpecifiedThenItIsInvalid) {
    const char *argv[] = {"cmag_browser", "my_file", "my_other_file"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
    EXPECT_FALSE(parser.getErrorMessage().empty());
}

TEST_F(BrowserArgumentParserTest, givenUnknownOptionSpecifiedThenItIsInvalid) {
    const char *argv[] = {"cmag_browser", "my_file", "-c"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    BrowserArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
    EXPECT_FALSE(parser.getErrorMessage().empty());
}

TEST_F(BrowserArgumentParserTest, givenVersionArgumentSpecifiedThenItIsProcessed) {
    testBooleanArg(
        "-v",
        [](const BrowserArgumentParser &parser) {
            return parser.getShowVersion();
        });
}

TEST_F(BrowserArgumentParserTest, givenDebugWidgetsArgumentSpecifiedThenItIsProcessed) {
    testBooleanArg(
        "-d",
        [](const BrowserArgumentParser &parser) {
            return parser.getShowDebugWidgets();
        });
}
