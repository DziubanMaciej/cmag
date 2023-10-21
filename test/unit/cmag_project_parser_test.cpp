#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/core/cmag_project_parser.h"

#include <gtest/gtest.h>

TEST(CmagProjectParserTest, givenProjectWithNoTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "targets" : []
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
    ASSERT_EQ(0u, project.getTargets().size());
}

TEST(CmagProjectParserTest, givenProjectWithDarkModeThenParseCorrectly) {
    {
        const char *json = R"DELIMETER(
        {
            "darkMode": true,
            "targets" : []
        }
        )DELIMETER";
        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
        EXPECT_TRUE(project.getDarkMode());
    }
    {
        const char *json = R"DELIMETER(
        {
            "darkMode": false,
            "targets" : []
        }
        )DELIMETER";
        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
        EXPECT_FALSE(project.getDarkMode());
    }
}

TEST(CmagProjectParserTest, givenTargetWithNoPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "targets" : [
            {
                "name": "myTarget",
                "type": "Executable",
                "properties": {}
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);
    EXPECT_EQ(0u, target.properties.size());
}

TEST(CmagProjectParserTest, givenTargetWithPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "targets" : [
            {
                "name": "myTarget",
                "type": "Executable",
                "properties": {
                    "one": "1",
                    "two": "2"
                }
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);
    ASSERT_EQ(2u, target.properties.size());
    EXPECT_STREQ("one", target.properties[0].first.c_str());
    EXPECT_STREQ("1", target.properties[0].second.c_str());
    EXPECT_STREQ("two", target.properties[1].first.c_str());
    EXPECT_STREQ("2", target.properties[1].second.c_str());
}

TEST(CmagProjectParserTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "targets" : [
            {
                "name": "myTarget1",
                "type": "Executable",
                "properties": {}
            },
            {
                "name": "myTarget2",
                "type": "Executable",
                "properties": {}
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
    ASSERT_EQ(2u, project.getTargets().size());

    const CmagTarget *target = &project.getTargets()[0];
    EXPECT_STREQ("myTarget1", target->name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target->type);
    EXPECT_EQ(0u, target->properties.size());

    target = &project.getTargets()[1];
    EXPECT_STREQ("myTarget2", target->name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target->type);
    EXPECT_EQ(0u, target->properties.size());
}

TEST(CmagProjectParserTest, givenVariousTargetTypesTheParseThemCorrectly) {
    std::pair<CmagTargetType, const char *> cases[] = {
        {CmagTargetType::StaticLibrary, "StaticLibrary"},
        {CmagTargetType::ModuleLibrary, "ModuleLibrary"},
        {CmagTargetType::SharedLibrary, "SharedLibrary"},
        {CmagTargetType::ObjectLibrary, "ObjectLibrary"},
        {CmagTargetType::InterfaceLibrary, "InterfaceLibrary"},
        {CmagTargetType::Executable, "Executable"},
    };

    for (auto [type, typeString] : cases) {
        const char *jsonFormat = R"DELIMETER(
        {
            "targets" : [
                {
                    "name": "myTarget",
                    "type": "%s",
                    "properties": {}
                }
            ]
        }
        )DELIMETER";
        char json[4096];
        sprintf(json, jsonFormat, typeString);

        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagProjectParser::parseProject(json, project));
        ASSERT_EQ(1u, project.getTargets().size());
        const CmagTarget &target = project.getTargets()[0];
        EXPECT_STREQ("myTarget", target.name.c_str());
        EXPECT_EQ(type, target.type);
        EXPECT_EQ(0u, target.properties.size());
    }
}


TEST(CmagProjectParserTest, givenTargetWithInvalidTypeThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "targets" : [
            {
                "name": "myTarget",
                "type": "exe",
                "properties": {}
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::InvalidValue, CmagProjectParser::parseProject(json, project));
}