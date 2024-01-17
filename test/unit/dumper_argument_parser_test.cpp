#include "cmag_core/dumper/dumper_argument_parser.h"

#include <gtest/gtest.h>

void compareArgs(const std::vector<std::string> &exp, const std::vector<std::string> &act) {
    ASSERT_EQ(exp.size(), act.size());
    for (size_t i = 0u; i < exp.size(); i++) {
        EXPECT_EQ(exp[i], act[i]);
    }
}

TEST(DumperArgumentParserTest, givenEmptyArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(DumperArgumentParserTest, givenNoCmakeArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(DumperArgumentParserTest, givenOnlyBuildPathArgumentThenSourcePathDefaultsToCwd) {
    const char *argv[] = {"cmag", "cmake", "-B", "build"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ(".", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("build", parser.getBuildPath().string().c_str());
}

TEST(DumperArgumentParserTest, givenSourcePathArgumentThenSourcePathIsValid) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenExplicitSourcePathArgumentThenSourcePathIsValid) {
    const char *argv[] = {"cmag", "cmake", "-S", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueArgumentWithEqualSignThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-S=.."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueArgumentWithoutEqualSignThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-S.."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenNoBuildPathArgumentThenBuildPathDefaultsToCurrentDirectory) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
    EXPECT_STREQ(".", parser.getBuildPath().string().c_str());
}

TEST(DumperArgumentParserTest, givenExplicitBuildPathArgumentThenBuildPathIsValid) {
    const char *argv[] = {"cmag", "cmake", "-S", "..", "-B", "buildDebug"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("buildDebug", parser.getBuildPath().string().c_str());
}

TEST(DumperArgumentParserTest, givenMultipleSourcePathArgumentsThenLastSourcePathIsUsed) {
    const char *argv[] = {"cmag", "cmake", "1", "2", "-S", "3", "4", "-S", "5", "6"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("6", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenOptionsBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-Wdev", "..", "--fresh"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueOptionsThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B", "build", "..", "-G", "Ninja", "-P", "script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueOptionsWithEqualsSignThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B=build", "..", "-G=Ninja", "-P=script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueOptionWithoutKeyThenArgumentsAreInvalid) {
    // This is a list of key-value args picked arbitrarily.
    const char *keyValueArgs[] = {
        "--graphviz",
        "-U",
        "--install-prefix",
    };
    for (const char *keyValueArg : keyValueArgs) {
        const char *argv[] = {"cmag", "cmake", "..", keyValueArg};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        DumperArgumentParser parser{argc, argv};
        EXPECT_FALSE(parser.isValid());
    }
}

TEST(DumperArgumentParserTest, givenKeyValueOptionWithEqualsSignAndWithoutKeyThenArgumentsAreInvalid) {
    // This is a list of key-value args picked arbitrarily.
    const char *keyValueArgs[] = {
        "--graphviz=",
        "-U=",
        "--install-prefix=",
    };
    for (const char *keyValueArg : keyValueArgs) {
        const char *argv[] = {"cmag", "cmake", "..", keyValueArg};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        DumperArgumentParser parser{argc, argv};
        EXPECT_FALSE(parser.isValid());
    }
}

TEST(DumperArgumentParserTest, givenDefinitionArgWhenParsingThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-DCMAKE_BUILD_TYPE=Debug", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenDefinitionArgWithSpaceWhenParsingThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-D", "CMAKE_BUILD_TYPE=Debug", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenNoExtraArgsWhenConstructingArgsForCMakeThenReturnArgumentsVerbatim) {
    const char *argv[] = {"cmag", "cmake", "-U", "1", "..", "--graphviz=a.graph"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
        "-U",
        "1",
        "..",
        "--graphviz=a.graph",
    };
    compareArgs(expectedArgs, cmakeArgs);
}

TEST(DumperArgumentParserTest, givenCmagArgsArePassedThenConstructCmakeArgsProperly) {
    const char *argv[] = {"cmag", "-p", "Aaa", "-e", "Bbb", "cmake", "-U", "1", "..", "--graphviz=a.graph"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
        "-U",
        "1",
        "..",
        "--graphviz=a.graph",
    };
    compareArgs(expectedArgs, cmakeArgs);
}

TEST(DumperArgumentParserTest, givenProjectNameIsNotPassedThenDefaultNameIsUsed) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("project", parser.getProjectName().c_str());
}

TEST(DumperArgumentParserTest, givenProjectNameIsPassedThenItIsUsed) {
    const char *argv[] = {"cmag", "-p", "Aaa", "-p", "Bbb", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("Bbb", parser.getProjectName().c_str());
}

TEST(DumperArgumentParserTest, givenExtraTargetPropertiesAreNotPassedThenListIsEmpty) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("", parser.getExtraTargetProperties().c_str());
}

TEST(DumperArgumentParserTest, givenExtraTargetPropertiesArePassedThenPopulateList) {
    const char *argv[] = {"cmag", "-e", "Aa;Bb", "-e", "Cc;Dd", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("Aa;Bb;Cc;Dd", parser.getExtraTargetProperties().c_str());
}

TEST(DumperArgumentParserTest, givenUnknownCmagArgThenReturnError) {
    const char *argv[] = {"cmag", "-x", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(DumperArgumentParserTest, givenEmptyArgsToCmakeThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "", "-S", "source", "", "-B", "build", ""};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("source", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("build", parser.getBuildPath().string().c_str());
}

TEST(DumperArgumentParserTest, givenNoJsonDebugArgThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_FALSE(parser.getJsonDebug());
}

TEST(DumperArgumentParserTest, givenJsonDebugArgThenParseCorrectly) {
    const char *argv[] = {"cmag", "-d", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_TRUE(parser.getJsonDebug());
}

TEST(DumperArgumentParserTest, givenLaunchGuiArgumentThenItIsParsedCorrectly) {
    {
        const char *argv[] = {"cmag", "cmake", ".."};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        DumperArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_FALSE(parser.getLaunchGui());
    }
    {
        const char *argv[] = {"cmag", "-g", "cmake", ".."};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        DumperArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_TRUE(parser.getLaunchGui());
    }
}
