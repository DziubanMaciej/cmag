#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/core/cmag_json_parser.h"

#include <gtest/gtest.h>

void compareTargetProperties(const CmagTarget::Properties &expected, const CmagTarget::Properties &actual) {
    // Config name
    EXPECT_STREQ(expected.first.c_str(), actual.first.c_str());

    // Properties
    ASSERT_EQ(expected.second.size(), actual.second.size());
    for (size_t i = 0u; i < expected.second.size(); i++) {
        EXPECT_STREQ(expected.second[i].first.c_str(), actual.second[i].first.c_str());
        EXPECT_STREQ(expected.second[i].second.c_str(), actual.second[i].second.c_str());
    }
}

TEST(CmagProjectParseTest, givenProjectWithNoTargetsAndNoGlobalsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : []
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(0u, project.getTargets().size());
    EXPECT_FALSE(project.getGlobals().darkMode);
}

TEST(CmagProjectParseTest, givenProjectWithDarkModeThenParseCorrectly) {
    {
        const char *json = R"DELIMETER(
        {
            "globals": {
                "darkMode": true
            },
            "targetsDebug" : []
        }
        )DELIMETER";
        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
        EXPECT_TRUE(project.getGlobals().darkMode);
    }
    {
        const char *json = R"DELIMETER(
        {
            "globals": {
                "darkMode": false
            },
            "targetsDebug" : []
        }
        )DELIMETER";
        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
        EXPECT_FALSE(project.getGlobals().darkMode);
    }
}

TEST(CmagProjectParseTest, givenTargetWithNoPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : [
            {
                "name": "myTarget",
                "type": "Executable",
                "properties": {}
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTarget::Properties expectedProperties = {
        "Debug",
        {},
    };
    ASSERT_EQ(1u, target.properties.size());
    compareTargetProperties(expectedProperties, target.properties[0]);
}

TEST(CmagProjectParseTest, givenTargetWithPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : [
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
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTarget::Properties expectedProperties = {
        "Debug",
        {
            {"one", "1"},
            {"two", "2"},
        },
    };
    ASSERT_EQ(1u, target.properties.size());
    compareTargetProperties(expectedProperties, target.properties[0]);
}

TEST(CmagProjectParseTest, givenTargetWithMultipleConfigsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : [
            {
                "name": "myTarget",
                "type": "Executable",
                "properties": {
                    "one": "1d",
                    "two": "2d"
                }
            }
        ],
        "targetsRelease" : [
            {
                "name": "myTarget",
                "type": "Executable",
                "properties": {
                    "one": "1r",
                    "two": "2r"
                }
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);
    ASSERT_EQ(2u, target.properties.size());

    ASSERT_EQ(2u, target.properties.size());
    {
        CmagTarget::Properties expectedProperties = {
            "Debug",
            {
                {"one", "1d"},
                {"two", "2d"},
            },
        };
        compareTargetProperties(expectedProperties, target.properties[0]);
    }
    {
        CmagTarget::Properties expectedProperties = {
            "Release",
            {
                {"one", "1r"},
                {"two", "2r"},
            },
        };
        compareTargetProperties(expectedProperties, target.properties[1]);
    }
}

TEST(CmagProjectParseTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : [
            {
                "name": "myTarget1",
                "type": "Executable",
                "properties": {
                    "prop": "1"
                }
            },
            {
                "name": "myTarget2",
                "type": "Executable",
                "properties": {
                    "prop": "2"
                }
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(2u, project.getTargets().size());

    {
        const CmagTarget &target = project.getTargets()[0];
        CmagTarget::Properties expectedProperties = {
            "Debug",
            {
                {"prop", "1"},
            },
        };
        ASSERT_EQ(1u, target.properties.size());
        compareTargetProperties(expectedProperties, target.properties[0]);
    }
    {
        const CmagTarget &target = project.getTargets()[1];
        CmagTarget::Properties expectedProperties = {
            "Debug",
            {
                {"prop", "2"},
            },
        };
        ASSERT_EQ(1u, target.properties.size());
        compareTargetProperties(expectedProperties, target.properties[0]);
    }
}

TEST(CmagProjectParseTest, givenVariousTargetTypesTheParseThemCorrectly) {
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
            "globals": {},
            "targetsDebug" : [
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
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
        ASSERT_EQ(1u, project.getTargets().size());
        const CmagTarget &target = project.getTargets()[0];
        EXPECT_STREQ("myTarget", target.name.c_str());
        EXPECT_EQ(type, target.type);

        CmagTarget::Properties expectedProperties = {
            "Debug",
            {},
        };
        ASSERT_EQ(1u, target.properties.size());
        compareTargetProperties(expectedProperties, target.properties[0]);
    }
}

TEST(CmagProjectParseTest, givenTargetWithInvalidTypeThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targetsDebug" : [
            {
                "name": "myTarget",
                "type": "exe",
                "properties": {}
            }
        ]
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::InvalidValue, CmagJsonParser::parseProject(json, project));
}
