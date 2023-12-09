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

TEST(DumperArgumentParserTest, givenKeyValueOptionsBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B", "build", "..", "-G", "Ninja", "-P", "script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenKeyValueOptionsWithEqualsSignBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B=build", "..", "-G=Ninja", "-P=script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(DumperArgumentParserTest, givenNoGraphvizOptionThenInsertOurOwn) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    fs::path expectedGraphvizPath = "./graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(DumperArgumentParserTest, givenNoGraphvizOptionAndExplicitBuildPathThenInsertOurOwnGraphvizPathRelativeToBuildPath) {
    const char *argv[] = {"cmag", "cmake", "..", "-B", "myBuild"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    fs::path expectedGraphvizPath = "myBuild/graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(DumperArgumentParserTest, givenGraphvizOptionThenParseIt) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz", "a.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("a.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(DumperArgumentParserTest, givenGraphvizOptionWithEqualsSignThenParseIt) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("a.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(DumperArgumentParserTest, givenMultipleGraphvizOptionsThenParseUseLastOne) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.dot", "--graphviz", "b.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("b.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(DumperArgumentParserTest, givenOptionStartingWithGraphvizThenIgnoreItAndParseNextArgs) {
    const char *argv[] = {"cmag", "cmake", "--graphvizz", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());

    fs::path expectedGraphvizPath = "./graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(DumperArgumentParserTest, givenUndefinedGraphvizOptionThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(DumperArgumentParserTest, givenUndefinedGraphvizOptionWithEqualsSignThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz="};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
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
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.graph", "-B", "bbb"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_TRUE(parser.getExtraArgs().empty());

    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
        "..",
        "--graphviz=a.graph",
        "-B",
        "bbb",
    };
    compareArgs(expectedArgs, cmakeArgs);
}

TEST(DumperArgumentParserTest, givenExtraArgsWhenConstructingArgsForCMakeThenAppendThemAtTheEnd) {
    const char *argv[] = {"cmag", "cmake", "..", "-B", "bbb"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_EQ(2u, parser.getExtraArgs().size());

    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
        "..",
        "-B",
        "bbb",
        "--graphviz",
        (fs::path("bbb") / "graph.dot").string(),
    };
    compareArgs(expectedArgs, cmakeArgs);
}

TEST(DumperArgumentParserTest, givenCmagArgsArePassedThenConstructCmakeArgsProperly) {
    const char *argv[] = {"cmag", "-p", "Aaa", "-e", "Bbb", "cmake", "..", "--graphviz=a.graph"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    DumperArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
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