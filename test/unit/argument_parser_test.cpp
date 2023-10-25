#include "cmag_lib/cmake_generation/argument_parser.h"

#include <gtest/gtest.h>

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
    EXPECT_EQ(6u, cmakeArgs.size());
    EXPECT_STREQ("cmake", cmakeArgs[0]);
    EXPECT_STREQ("..", cmakeArgs[1]);
    EXPECT_STREQ("--graphviz=a.graph", cmakeArgs[2]);
    EXPECT_STREQ("-B", cmakeArgs[3]);
    EXPECT_STREQ("bbb", cmakeArgs[4]);
    EXPECT_EQ(nullptr, cmakeArgs[5]);
}

TEST(ArgumentParserTest, givenExtraArgsWhenConstructingArgsForCMakeThenReturnAppendThemAtTheEnd) {
    const char *argv[] = {"cmag", "cmake", "..", "-B", "bbb"};
    const int argc = sizeof(argv) / sizeof(argv[0]);
    ArgumentParser parser{argc, argv};
    EXPECT_TRUE(parser.isValid());
    EXPECT_EQ(2u, parser.getExtraArgs().size());

    auto cmakeArgs = parser.constructArgsForCmake();
    EXPECT_EQ(7u, cmakeArgs.size());
    EXPECT_STREQ("cmake", cmakeArgs[0]);
    EXPECT_STREQ("..", cmakeArgs[1]);
    EXPECT_STREQ("-B", cmakeArgs[2]);
    EXPECT_STREQ("bbb", cmakeArgs[3]);
    EXPECT_STREQ("--graphviz", cmakeArgs[4]);
    EXPECT_EQ(fs::path("bbb/graph.dot"), cmakeArgs[5]);
    EXPECT_EQ(nullptr, cmakeArgs[6]);
}
