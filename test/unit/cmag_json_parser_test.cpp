#include "cmag_lib/core/cmag_json_parser.h"
#include "cmag_lib/core/cmag_project.h"

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
                "type": "EXECUTABLE",
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
                "type": "EXECUTABLE",
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
                "type": "EXECUTABLE",
                "properties": {
                    "one": "1d",
                    "two": "2d"
                }
            }
        ],
        "targetsRelease" : [
            {
                "name": "myTarget",
                "type": "EXECUTABLE",
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
                "type": "EXECUTABLE",
                "properties": {
                    "prop": "1"
                }
            },
            {
                "name": "myTarget2",
                "type": "EXECUTABLE",
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
        {CmagTargetType::StaticLibrary, "STATIC_LIBRARY"},
        {CmagTargetType::ModuleLibrary, "MODULE_LIBRARY"},
        {CmagTargetType::SharedLibrary, "SHARED_LIBRARY"},
        {CmagTargetType::ObjectLibrary, "OBJECT_LIBRARY"},
        {CmagTargetType::InterfaceLibrary, "INTERFACE_LIBRARY"},
        {CmagTargetType::Executable, "EXECUTABLE"},
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

TEST(CmagConfigListFileParseTest, givenEmptyConfigsListThenParseCorrectly) {
    const char *json = R"DELIMETER(
    []
    )DELIMETER";
    std::vector<std::string> configs{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseConfigListFile(json, configs));
    ASSERT_EQ(0u, configs.size());
}

TEST(CmagConfigListFileParseTest, givenOneConfigThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [ "Debug" ]
    )DELIMETER";
    std::vector<std::string> configs{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseConfigListFile(json, configs));
    ASSERT_EQ(1u, configs.size());
    EXPECT_STREQ("Debug", configs[0].c_str());
}

TEST(CmagConfigListFileParseTest, givenMultipleConfigsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [ "Debug", "Release", "CustomConf" ]
    )DELIMETER";
    std::vector<std::string> configs{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseConfigListFile(json, configs));
    ASSERT_EQ(3u, configs.size());
    EXPECT_STREQ("Debug", configs[0].c_str());
    EXPECT_STREQ("Release", configs[1].c_str());
    EXPECT_STREQ("CustomConf", configs[2].c_str());
}

TEST(CmagGlobalsFileParseTest, givenEmptyGlobalsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {}
    )DELIMETER";
    CmagGlobals globals{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseGlobalsFile(json, globals));
    EXPECT_FALSE(globals.darkMode);
}

TEST(CmagGlobalsFileParseTest, givenDarkModeSpecifiedThenParseCorrectly) {
    {
        const char *json = R"DELIMETER(
        {
            "darkMode" : false
        }
        )DELIMETER";
        CmagGlobals globals{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseGlobalsFile(json, globals));
        EXPECT_FALSE(globals.darkMode);
    }
    {
        const char *json = R"DELIMETER(
        {
            "darkMode" : true
        }
        )DELIMETER";
        CmagGlobals globals{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseGlobalsFile(json, globals));
        EXPECT_TRUE(globals.darkMode);
    }
}

TEST(CmagTargetsFileParseTest, givenNoTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "config": "Debug",
        "targets": []
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(0u, targets.size());
}

TEST(CmagTargetsFileParseTest, givenTargetWithNoPropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "config": "Debug",
        "targets": [
            {
                "name": "myTarget",
                "type": "EXECUTABLE",
                "properties": { }
            }
        ]
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(1u, targets.size());

    const CmagTarget &target = targets[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTarget::Properties expectedProperties = {
        "Debug",
        {},
    };
    ASSERT_EQ(1u, target.properties.size());
    compareTargetProperties(expectedProperties, target.properties[0]);
}

TEST(CmagTargetsFileParseTest, givenTargetWithPropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "config": "Debug",
        "targets": [
            {
                "name": "myTarget",
                "type": "EXECUTABLE",
                "properties": {
                    "one": "1",
                    "two": "2"
                }
            }
        ]
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(1u, targets.size());

    const CmagTarget &target = targets[0];
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

TEST(CmagTargetsFileParseTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "config": "Debug",
        "targets": [
            {
                "name": "myTarget1",
                "type": "EXECUTABLE",
                "properties": {
                    "one": "1",
                    "two": "2"
                }
            },
            {
                "name": "myTarget2",
                "type": "EXECUTABLE",
                "properties": {
                    "three": "3",
                    "zfour": "4"
                }
            }
        ]
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(2u, targets.size());

    {
        const CmagTarget &target = targets[0];
        EXPECT_STREQ("myTarget1", target.name.c_str());
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
    {
        const CmagTarget &target = targets[1];
        EXPECT_STREQ("myTarget2", target.name.c_str());
        EXPECT_EQ(CmagTargetType::Executable, target.type);

        CmagTarget::Properties expectedProperties = {
            "Debug",
            {
                {"three", "3"},
                {"zfour", "4"}, // nlohmann's json library sorts alphabetically by key. We don't care in normal code, but for tests we have to maintain order
            },
        };
        ASSERT_EQ(1u, target.properties.size());
        compareTargetProperties(expectedProperties, target.properties[0]);
    }
}
