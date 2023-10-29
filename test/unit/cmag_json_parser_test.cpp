#include "cmag_lib/core/cmag_project.h"
#include "cmag_lib/json/cmag_json_parser.h"

#include <gtest/gtest.h>

void compareTargetProperties(const CmagTargetConfig &expected, const CmagTargetConfig &actual) {
    // Config name
    EXPECT_STREQ(expected.name.c_str(), actual.name.c_str());

    // Properties
    ASSERT_EQ(expected.properties.size(), actual.properties.size());
    for (size_t i = 0u; i < expected.properties.size(); i++) {
        EXPECT_STREQ(expected.properties[i].name.c_str(), actual.properties[i].name.c_str());
        EXPECT_STREQ(expected.properties[i].value.c_str(), actual.properties[i].value.c_str());
    }
}

TEST(CmagProjectParseTest, givenProjectWithNoTargetsAndNoGlobalsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {}
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
            "targets" : {}
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
            "targets" : {}
        }
        )DELIMETER";
        CmagProject project{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
        EXPECT_FALSE(project.getGlobals().darkMode);
    }
}

TEST(CmagProjectParseTest, givenTargetWithNoConfigsThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {}
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::MissingField, CmagJsonParser::parseProject(json, project));
}

TEST(CmagProjectParseTest, givenTargetWithNoPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug": {}
                },
                "graphical": {
                    "x": 10,
                    "y": 10
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTargetConfig expectedProperties = {
        "Debug",
        {},
    };
    ASSERT_EQ(1u, target.configs.size());
    compareTargetProperties(expectedProperties, target.configs[0]);
}

TEST(CmagProjectParseTest, givenTargetWithPropertiesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug": {
                        "one": "1",
                        "two": "2"
                    }
                },
                "graphical": {
                    "x": 10,
                    "y": 10
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTargetConfig expectedProperties = {
        "Debug",
        {
            {"one", "1"},
            {"two", "2"},
        },
    };
    ASSERT_EQ(1u, target.configs.size());
    compareTargetProperties(expectedProperties, target.configs[0]);
}

TEST(CmagProjectParseTest, givenTargetWithMultipleConfigsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug" : {
                        "one": "1d",
                        "two": "2d"
                    },
                    "Release" : {
                        "one": "1r",
                        "two": "2r"
                    }
                },
                "graphical": {
                    "x": 10,
                    "y": 10
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);
    ASSERT_EQ(2u, target.configs.size());

    ASSERT_EQ(2u, target.configs.size());
    {
        CmagTargetConfig expectedProperties = {
            "Debug",
            {
                {"one", "1d"},
                {"two", "2d"},
            },
        };
        compareTargetProperties(expectedProperties, target.configs[0]);
    }
    {
        CmagTargetConfig expectedProperties = {
            "Release",
            {
                {"one", "1r"},
                {"two", "2r"},
            },
        };
        compareTargetProperties(expectedProperties, target.configs[1]);
    }
}

TEST(CmagProjectParseTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget1" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug" : {
                        "prop": "1"
                    }
                },
                "graphical": {
                    "x": 10,
                    "y": 10
                }
            },
            "myTarget2" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug" : {
                        "prop": "2"
                    }
                },
                "graphical": {
                    "x": 10,
                    "y": 10
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json, project));
    ASSERT_EQ(2u, project.getTargets().size());

    {
        const CmagTarget &target = project.getTargets()[0];
        CmagTargetConfig expectedProperties = {
            "Debug",
            {
                {"prop", "1"},
            },
        };
        ASSERT_EQ(1u, target.configs.size());
        compareTargetProperties(expectedProperties, target.configs[0]);
    }
    {
        const CmagTarget &target = project.getTargets()[1];
        CmagTargetConfig expectedProperties = {
            "Debug",
            {
                {"prop", "2"},
            },
        };
        ASSERT_EQ(1u, target.configs.size());
        compareTargetProperties(expectedProperties, target.configs[0]);
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
            "targets" : {
                "myTarget" : {
                    "type": "%s",
                    "configs": {
                        "Debug" : {}
                    },
                    "graphical": {
                        "x": 10,
                        "y": 10
                    }
                }
            }
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

        CmagTargetConfig expectedProperties = {
            "Debug",
            {},
        };
        ASSERT_EQ(1u, target.configs.size());
        compareTargetProperties(expectedProperties, target.configs[0]);
    }
}

TEST(CmagProjectParseTest, givenTargetWithInvalidTypeThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "myTarget" : {
                "type": "exe",
                "configs": {
                    "Debug" : {}
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::InvalidValue, CmagJsonParser::parseProject(json, project));
}

TEST(CmagProjectParseTest, givenTargetWithEmptyNameThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "globals": {},
        "targets" : {
            "" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug" : {}
                }
            }
        }
    }
    )DELIMETER";
    CmagProject project{};
    ASSERT_EQ(ParseResult::InvalidValue, CmagJsonParser::parseProject(json, project));
}

TEST(CmagTargetsFilesListFileParseTest, givenEmptyConfigsListThenParseCorrectly) {
    const char *json = R"DELIMETER(
    []
    )DELIMETER";
    std::vector<fs::path> files{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFilesListFile(json, files));
    ASSERT_EQ(0u, files.size());
}

TEST(CmagTargetsFilesListFileParseTest, givenOneConfigThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [ "project_Debug.cmag-targets-list" ]
    )DELIMETER";
    std::vector<fs::path> files{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFilesListFile(json, files));
    ASSERT_EQ(1u, files.size());
    EXPECT_EQ("project_Debug.cmag-targets-list", files[0]);
}

TEST(CmagTargetsFilesListFileParseTest, givenMultipleConfigsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [
        "project_Debug.cmag-targets-list",
        "project_Release.cmag-targets-list",
        "project_CustomConf.cmag-targets-list"
    ]
    )DELIMETER";
    std::vector<fs::path> files{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFilesListFile(json, files));
    ASSERT_EQ(3u, files.size());
    EXPECT_EQ("project_Debug.cmag-targets-list", files[0]);
    EXPECT_EQ("project_Release.cmag-targets-list", files[1]);
    EXPECT_EQ("project_CustomConf.cmag-targets-list", files[2]);
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
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(0u, targets.size());
}

TEST(CmagTargetsFileParseTest, givenTargetWithNoConfigsThenReturnError) {
    const char *json = R"DELIMETER(
    {
        "myTarget" : {
            "type": "EXECUTABLE",
            "configs": { }
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::MissingField, CmagJsonParser::parseTargetsFile(json, targets));
}

TEST(CmagTargetsFileParseTest, givenTargetWithNoPropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget" : {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {}
            }
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(1u, targets.size());

    const CmagTarget &target = targets[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTargetConfig expectedProperties = {
        "Debug",
        {},
    };
    ASSERT_EQ(1u, target.configs.size());
    compareTargetProperties(expectedProperties, target.configs[0]);
}

TEST(CmagTargetsFileParseTest, givenTargetWithPropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget": {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "one": "1",
                    "two": "2"
                }
            }
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(1u, targets.size());

    const CmagTarget &target = targets[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTargetConfig expectedProperties = {
        "Debug",
        {
            {"one", "1"},
            {"two", "2"},
        },
    };
    ASSERT_EQ(1u, target.configs.size());
    compareTargetProperties(expectedProperties, target.configs[0]);
}

TEST(CmagTargetsFileParseTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget1" : {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "one": "1",
                    "two": "2"
                }
            }
        },
        "myTarget2" : {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "three": "3",
                    "zfour": "4"
                }
            }
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseTargetsFile(json, targets));
    ASSERT_EQ(2u, targets.size());

    {
        const CmagTarget &target = targets[0];
        EXPECT_STREQ("myTarget1", target.name.c_str());
        EXPECT_EQ(CmagTargetType::Executable, target.type);

        CmagTargetConfig expectedProperties = {
            "Debug",
            {
                {"one", "1"},
                {"two", "2"},
            },
        };
        ASSERT_EQ(1u, target.configs.size());
        compareTargetProperties(expectedProperties, target.configs[0]);
    }
    {
        const CmagTarget &target = targets[1];
        EXPECT_STREQ("myTarget2", target.name.c_str());
        EXPECT_EQ(CmagTargetType::Executable, target.type);

        CmagTargetConfig expectedProperties = {
            "Debug",
            {
                {"three", "3"},
                {"zfour", "4"}, // nlohmann's json library sorts alphabetically by key. We don't care in normal code, but for tests we have to maintain order
            },
        };
        ASSERT_EQ(1u, target.configs.size());
        compareTargetProperties(expectedProperties, target.configs[0]);
    }
}
