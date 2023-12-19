#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

TEST(NlohmannJsonTest, EmptyStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    ""
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("", str.c_str());
}

TEST(NlohmannJsonTest, OneLetterStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    "a"
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("a", str.c_str());
}

TEST(NlohmannJsonTest, ComplicatedStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    "abc 'hello \"world\""
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("abc 'hello \"world\"", str.c_str());
}

TEST(NlohmannJsonTest, EmptyRawStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    ''''''
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("", str.c_str());
}

TEST(NlohmannJsonTest, OneLetterRawStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    '''a'''
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("a", str.c_str());
}

TEST(NlohmannJsonTest, ComplicatedRawStringIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    '''abc 'hello "world"'''
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("abc 'hello \"world\"", str.c_str());
}

TEST(NlohmannJsonTest, RawStringWithApostrophesIsParsedCorrectly) {
    const char *json = R"DELIMETER(
    '''a ' b '' c ' d'''
    )DELIMETER";

    const nlohmann::json node = nlohmann::json::parse(json, nullptr, false);
    EXPECT_FALSE(node.is_discarded());
    std::string str = node.get<std::string>();
    EXPECT_STREQ("a ' b '' c ' d", str.c_str());
}
