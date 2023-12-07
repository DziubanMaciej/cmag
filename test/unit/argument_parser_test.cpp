#include "cmag_core/core/argument_parser.h"

#include <gtest/gtest.h>

void compareArgs(const std::vector<std::string> &exp, const std::vector<std::string> &act) {
    ASSERT_EQ(exp.size(), act.size());
    for (size_t i = 0u; i < exp.size(); i++) {
        EXPECT_EQ(exp[i], act[i]);
    }
}

TEST(ArgumentParserTest, givenEmptyArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(ArgumentParserTest, givenNoCmakeArgumentsThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(ArgumentParserTest, givenOnlyBuildPathArgumentThenSourcePathDefaultsToCwd) {
    const char *argv[] = {"cmag", "cmake", "-B", "build"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ(".", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("build", parser.getBuildPath().string().c_str());
}

TEST(ArgumentParserTest, givenSourcePathArgumentThenSourcePathIsValid) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenExplicitSourcePathArgumentThenSourcePathIsValid) {
    const char *argv[] = {"cmag", "cmake", "-S", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenKeyValueArgumentWithEqualSignThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-S=.."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenKeyValueArgumentWithoutEqualSignThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", "-S.."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenNoBuildPathArgumentThenBuildPathDefaultsToCurrentDirectory) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
    EXPECT_STREQ(".", parser.getBuildPath().string().c_str());
}

TEST(ArgumentParserTest, givenExplicitBuildPathArgumentThenBuildPathIsValid) {
    const char *argv[] = {"cmag", "cmake", "-S", "..", "-B", "buildDebug"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("buildDebug", parser.getBuildPath().string().c_str());
}

TEST(ArgumentParserTest, givenMultipleSourcePathArgumentsThenLastSourcePathIsUsed) {
    const char *argv[] = {"cmag", "cmake", "1", "2", "-S", "3", "4", "-S", "5", "6"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("6", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenOptionsBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-Wdev", "..", "--fresh"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenKeyValueOptionsBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B", "build", "..", "-G", "Ninja", "-P", "script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenKeyValueOptionsWithEqualsSignBeyondSourcePathThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "-B=build", "..", "-G=Ninja", "-P=script"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());
}

TEST(ArgumentParserTest, givenNoGraphvizOptionThenInsertOurOwn) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    fs::path expectedGraphvizPath = "./graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(ArgumentParserTest, givenNoGraphvizOptionAndExplicitBuildPathThenInsertOurOwnGraphvizPathRelativeToBuildPath) {
    const char *argv[] = {"cmag", "cmake", "..", "-B", "myBuild"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());

    fs::path expectedGraphvizPath = "myBuild/graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(ArgumentParserTest, givenGraphvizOptionThenParseIt) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz", "a.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("a.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(ArgumentParserTest, givenGraphvizOptionWithEqualsSignThenParseIt) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("a.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(ArgumentParserTest, givenMultipleGraphvizOptionsThenParseUseLastOne) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.dot", "--graphviz", "b.dot"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("b.dot", parser.getGraphvizPath().string().c_str());
    EXPECT_TRUE(parser.getExtraArgs().empty());
}

TEST(ArgumentParserTest, givenOptionStartingWithGraphvizThenIgnoreItAndParseNextArgs) {
    const char *argv[] = {"cmag", "cmake", "--graphvizz", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("..", parser.getSourcePath().string().c_str());

    fs::path expectedGraphvizPath = "./graph.dot";
    EXPECT_EQ(expectedGraphvizPath, parser.getGraphvizPath());
    ASSERT_EQ(2u, parser.getExtraArgs().size());
    EXPECT_STREQ("--graphviz", parser.getExtraArgs()[0].c_str());
    EXPECT_EQ(expectedGraphvizPath, parser.getExtraArgs()[1]);
}

TEST(ArgumentParserTest, givenUndefinedGraphvizOptionThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(ArgumentParserTest, givenUndefinedGraphvizOptionWithEqualsSignThenArgumentsAreInvalid) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz="};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(ArgumentParserTest, givenNoExtraArgsWhenConstructingArgsForCMakeThenReturnArgumentsVerbatim) {
    const char *argv[] = {"cmag", "cmake", "..", "--graphviz=a.graph", "-B", "bbb"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
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

TEST(ArgumentParserTest, givenExtraArgsWhenConstructingArgsForCMakeThenReturnAppendThemAtTheEnd) {
    const char *argv[] = {"cmag", "cmake", "..", "-B", "bbb"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
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

TEST(ArgumentParserTest, givenCmagArgsArePassedThenConstructCmakeArgsProperly) {
    const char *argv[] = {"cmag", "-p", "Aaa", "-e", "Bbb", "cmake", "..", "--graphviz=a.graph"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    auto cmakeArgs = parser.constructArgsForCmake();
    std::vector<std::string> expectedArgs = {
        "cmake",
        "..",
        "--graphviz=a.graph",
    };
    compareArgs(expectedArgs, cmakeArgs);
}

TEST(ArgumentParserTest, givenProjectNameIsNotPassedThenDefaultNameIsUsed) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("project", parser.getProjectName().c_str());
}

TEST(ArgumentParserTest, givenProjectNameIsPassedThenItIsUsed) {
    const char *argv[] = {"cmag", "-p", "Aaa", "-p", "Bbb", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("Bbb", parser.getProjectName().c_str());
}

TEST(ArgumentParserTest, givenExtraTargetPropertiesAreNotPassedThenListIsEmpty) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("", parser.getExtraTargetProperties().c_str());
}

TEST(ArgumentParserTest, givenExtraTargetPropertiesArePassedThenPopulateList) {
    const char *argv[] = {"cmag", "-e", "Aa;Bb", "-e", "Cc;Dd", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("Aa;Bb;Cc;Dd", parser.getExtraTargetProperties().c_str());
}

TEST(ArgumentParserTest, givenUnknownCmagArgThenReturnError) {
    const char *argv[] = {"cmag", "-x", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_FALSE(parser.isValid());
}

TEST(ArgumentParserTest, givenEmptyArgsToCmakeThenIgnoreThem) {
    const char *argv[] = {"cmag", "cmake", "", "-S", "source", "", "-B", "build", ""};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_STREQ("source", parser.getSourcePath().string().c_str());
    EXPECT_STREQ("build", parser.getBuildPath().string().c_str());
}

TEST(ArgumentParserTest, givenNoJsonDebugArgThenParseCorrectly) {
    const char *argv[] = {"cmag", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_FALSE(parser.getJsonDebug());
}

TEST(ArgumentParserTest, givenJsonDebugArgThenParseCorrectly) {
    const char *argv[] = {"cmag", "-d", "cmake", ".."};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_TRUE(parser.getJsonDebug());
}

TEST(ArgumentParserTest, givenLaunchGuiArgumentThenItIsParsedCorrectly) {
    {
        const char *argv[] = {"cmag", "cmake", ".."};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        ArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_FALSE(parser.getLaunchGui());
    }
    {
        const char *argv[] = {"cmag", "-g", "cmake", ".."};
        const int argc = sizeof(argv) / sizeof(argv[0]);
        ArgumentParser parser{argc, argv};
        EXPECT_TRUE(parser.isValid());
        EXPECT_TRUE(parser.getLaunchGui());
    }
}
