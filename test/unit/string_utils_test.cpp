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