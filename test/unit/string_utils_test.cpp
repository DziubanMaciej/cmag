#include "cmag_core/utils/string_utils.h"

#include <gtest/gtest.h>

TEST(SplitCmakeListStringTest, givenEmptyStringThenReturnEmptyList) {
    EXPECT_TRUE(splitCmakeListString("", false).empty());
    EXPECT_TRUE(splitCmakeListString("", true).empty());
}

TEST(SplitCmakeListStringTest, givenNonListStringThenReturnOneEntry) {
    auto result = splitCmakeListString("Abcdef", false);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ("Abcdef", result[0]);
}

TEST(SplitCmakeListStringTest, givenNonListStringAndIgnoreSingleEntryThenReturnEmptyList) {
    auto result = splitCmakeListString("Abcdef", true);
    EXPECT_TRUE(result.empty());
}

TEST(SplitCmakeListStringTest, givenListStringThenReturnAllEntries) {
    auto result = splitCmakeListString("abc;def;ghi", false);
    ASSERT_EQ(3u, result.size());
    EXPECT_EQ("abc", result[0]);
    EXPECT_EQ("def", result[1]);
    EXPECT_EQ("ghi", result[2]);
}

TEST(SplitCmakeListStringTest, givenListStringAndIgnoreSingleEntryThenReturnAllEntries) {
    auto result = splitCmakeListString("abc;def;ghi", true);
    ASSERT_EQ(3u, result.size());
    EXPECT_EQ("abc", result[0]);
    EXPECT_EQ("def", result[1]);
    EXPECT_EQ("ghi", result[2]);
}

TEST(IsValidCmakeTargetNameTest, givenNormalAlphanumericNamesWhenCheckingValidityThenReturnTrue) {
    EXPECT_TRUE(isValidCmakeTargetName("a", true));
    EXPECT_TRUE(isValidCmakeTargetName("ABC", true));
    EXPECT_TRUE(isValidCmakeTargetName("aBc", true));
    EXPECT_TRUE(isValidCmakeTargetName("ZzaA", true));
}

TEST(IsValidCmakeTargetNameTest, givenNamesWithAllowedSpecialCharactersCheckingValidityThenReturnTrue) {
    EXPECT_TRUE(isValidCmakeTargetName("a++", true));
    EXPECT_TRUE(isValidCmakeTargetName("+B_1.+-_A", true));
    EXPECT_TRUE(isValidCmakeTargetName("1+B_1.+-_A", true));
}

TEST(IsValidCmakeTargetNameTest, givenEmptyNameWhenCheckingValidityThenReturnFalse) {
    EXPECT_FALSE(isValidCmakeTargetName("", true));
}

TEST(IsValidCmakeTargetNameTest, givenNamesWithColonsThenReturnTrueWhenTheyAreAllowed) {
    const char *names[] = {
        "abc::abc",
        ":a:b:c:",
        "abc::",
        "::abc",
        "abc:::abc",
        ":::::::", // seriously, this is legal
    };

    for (const char *name : names) {
        EXPECT_TRUE(isValidCmakeTargetName(name, true));
        EXPECT_FALSE(isValidCmakeTargetName(name, false));
    }
}

TEST(JoinStringWithCharTest, givenNormalStringsThenJoinProperly) {
    std::vector<std::string> arg = {
        "abc",
        "a",
        "def",
    };
    EXPECT_STREQ("abc;a;def", joinStringWithChar(arg, ';').c_str());
}

TEST(JoinStringWithCharTest, givenEmptyVectorThenReturnEmptyString) {
    std::vector<std::string> arg = {};
    EXPECT_STREQ("", joinStringWithChar(arg, ';').c_str());
}

TEST(JoinStringWithCharTest, givenEmptyStringsThenJoinProperly) {
    std::vector<std::string> arg = {
        "",
        "",
        "abc",
        "a",
        "",
        "def",
        "",
    };
    EXPECT_STREQ("abc;a;def", joinStringWithChar(arg, ';').c_str());
}

TEST(JoinStringWithCharTest, givenOnlyEmptyStringsThenReturnEmptyString) {
    std::vector<std::string> arg = {
        "",
        "",
        "",
    };
    EXPECT_STREQ("", joinStringWithChar(arg, ';').c_str());
}
