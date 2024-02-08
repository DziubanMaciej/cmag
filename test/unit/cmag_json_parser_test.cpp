#include "cmag_core/core/cmag_project.h"
#include "cmag_core/parse/cmag_json_parser.h"

#include <gtest/gtest.h>

struct CmagParseTest : ::testing::Test {
    static std::string constructGlobals(CmagVersion &version) {
        const char format[] = R"DELIMETER(
            {
                "darkMode": false,

                "selectedConfig": "",
                "cmagVersion": "%s",
                "cmakeVersion": "",
                "cmakeProjectName": "",
                "cmagProjectName": "",
                "sourceDir": "",
                "buildDir": "",
                "generator": "",
                "compilerId": "",
                "compilerVersion": "",
                "os": "",
                "useFolders": "",
                "browser": {
                    "needsLayout": true,
                    "autoSaveEnabled": true,
                    "cameraX": 0.0,
                    "cameraY": 0.0,
                    "cameraScale": 0.0,
                    "displayedDependencyType": 5,
                    "selectedTabIndex": 0,
                    "selectedTargetName": ""
                },
                "listDirs": { "a": [] }
            }
        )DELIMETER";

        char buffer[sizeof(format) + CmagVersion::maxStringLength];
        int r = snprintf(buffer, sizeof(buffer), format, version.toString().c_str());
        EXPECT_LE(0, r);
        return std::string{buffer};
    }

    template <typename... Args>
    const char *insertGlobals(const char *jsonFormat, CmagVersion version = cmagVersion, Args... args) {
        std::string globals = constructGlobals(version);
        int r = snprintf(jsonBuffer, sizeof(jsonBuffer), jsonFormat, globals.c_str(), args...);
        EXPECT_LE(0, r);
        return jsonBuffer;
    }

    char jsonBuffer[4096];
};

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

using CmagProjectParseTest = CmagParseTest;

TEST_F(CmagProjectParseTest, givenProjectWithNoTargetsAndEmptyGlobalsThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {}
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
    ASSERT_EQ(0u, project.getTargets().size());
    EXPECT_FALSE(project.getGlobals().darkMode);
}

TEST_F(CmagProjectParseTest, givenProjectWithIncompatibleVersionThenFailParsing) {
    CmagVersion version = ::cmagVersion;
    version.comp1++;
    const char *json = insertGlobals(
        R"DELIMETER(
    {
        "globals": %s,
        "targets" : {}
    }
    )DELIMETER",
        version);
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::VersionMismatch, CmagJsonParser::parseProject(json, project).status);
}

TEST_F(CmagProjectParseTest, givenProjectWithDifferentButCompatibleVersionThenSucceed) {
    CmagVersion version = ::cmagVersion;
    version.comp2++;
    const char *json = insertGlobals(
        R"DELIMETER(
    {
        "globals": %s,
        "targets" : {}
    }
    )DELIMETER",
        version);
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
}

TEST_F(CmagProjectParseTest, givenProjectWithInvalidStructureButIncompatibleCmagVersionPresentThenParseTheVersionAndFailParsing) {
    CmagVersion version = ::cmagVersion;
    version.comp0++;

    const char jsonFormat[] = R"DELIMETER(
    {
        "globals": {
            "cmagVersion": "%s"
        }
    }
    )DELIMETER";

    char json[sizeof(jsonFormat) + CmagVersion::maxStringLength];
    int r = snprintf(json, sizeof(json), jsonFormat, version.toString().c_str());
    ASSERT_LE(0, r);

    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::VersionMismatch, CmagJsonParser::parseProject(json, project).status);
}

TEST_F(CmagProjectParseTest, givenTargetWithNoConfigsThenReturnError) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {}
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::MissingField, CmagJsonParser::parseProject(json, project).status);
}

TEST_F(CmagProjectParseTest, givenTargetWithNoPropertiesThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug": {}
                },
                "graphical": {
                    "x": 10,
                    "y": 20,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": []
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
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

    EXPECT_EQ(10, target.graphical.x);
    EXPECT_EQ(20, target.graphical.y);

    EXPECT_STREQ("a", target.listDirName.c_str());
}

TEST_F(CmagProjectParseTest, givenTargetWithPropertiesThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
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
                    "y": 10,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": []
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
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

TEST_F(CmagProjectParseTest, givenTargetWithMultipleConfigsThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
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
                    "y": 10,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": []
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
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

TEST_F(CmagProjectParseTest, givenMultipleTargetsThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
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
                    "y": 10,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": []
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
                    "y": 10,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": []
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
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

TEST_F(CmagProjectParseTest, givenVariousTargetTypesTheParseThemCorrectly) {
    std::pair<CmagTargetType, const char *> cases[] = {
        {CmagTargetType::StaticLibrary, "STATIC_LIBRARY"},
        {CmagTargetType::ModuleLibrary, "MODULE_LIBRARY"},
        {CmagTargetType::SharedLibrary, "SHARED_LIBRARY"},
        {CmagTargetType::ObjectLibrary, "OBJECT_LIBRARY"},
        {CmagTargetType::InterfaceLibrary, "INTERFACE_LIBRARY"},
        {CmagTargetType::UnknownLibrary, "UNKNOWN_LIBRARY"},
        {CmagTargetType::UnknownTarget, "UNKNOWN"},
        {CmagTargetType::Utility, "UTILITY"},
        {CmagTargetType::Executable, "EXECUTABLE"},
    };
    static_assert(sizeof(cases) / sizeof(cases[0]) == static_cast<int>(CmagTargetType::COUNT) - 1); // do not parse INVALID, hence the minus one

    for (auto [type, typeString] : cases) {
        const char *json = insertGlobals(R"DELIMETER(
        {
            "globals": %s,
            "targets" : {
                "myTarget" : {
                    "type": "%s",
                    "configs": {
                        "Debug" : {}
                    },
                    "graphical": {
                        "x": 10,
                        "y": 10,
                        "hideConnections": false
                    },
                    "listDir": "a",
                    "isImported": false,
                    "aliases": []
                }
            }
        }
        )DELIMETER",
                                         cmagVersion, typeString);

        CmagProject project{};
        ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
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

TEST_F(CmagProjectParseTest, givenTargetWithAliasesThenParseCorrectly) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {
            "myTarget" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug": {}
                },
                "graphical": {
                    "x": 10,
                    "y": 20,
                    "hideConnections": false
                },
                "listDir": "a",
                "isImported": false,
                "aliases": [ "A", "B", "C"]
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseProject(json, project).status);
    ASSERT_EQ(1u, project.getTargets().size());
    const CmagTarget &target = project.getTargets()[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ((std::vector<std::string>{"A", "B", "C"}), target.aliases);
}

TEST_F(CmagProjectParseTest, givenTargetWithInvalidTypeThenReturnError) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {
            "myTarget" : {
                "type": "exe",
                "configs": {
                    "Debug" : {}
                }
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::InvalidValue, CmagJsonParser::parseProject(json, project).status);
}

TEST_F(CmagProjectParseTest, givenTargetWithEmptyNameThenReturnError) {
    const char *json = insertGlobals(R"DELIMETER(
    {
        "globals": %s,
        "targets" : {
            "" : {
                "type": "EXECUTABLE",
                "configs": {
                    "Debug" : {}
                }
            }
        }
    }
    )DELIMETER");
    CmagProject project{};
    ASSERT_EQ(ParseResultStatus::InvalidValue, CmagJsonParser::parseProject(json, project).status);
}

TEST(CmagTargetsFilesListFileParseTest, givenEmptyConfigsListThenParseCorrectly) {
    const char *json = R"DELIMETER(
    []
    )DELIMETER";
    std::vector<std::string> fileNames{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFilesListFile(json, fileNames).status);
    ASSERT_EQ(0u, fileNames.size());
}

TEST(CmagTargetsFilesListFileParseTest, givenOneConfigThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [ "project_Debug.cmag-targets-list" ]
    )DELIMETER";
    std::vector<std::string> fileNames{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFilesListFile(json, fileNames).status);
    ASSERT_EQ(1u, fileNames.size());
    EXPECT_EQ("project_Debug.cmag-targets-list", fileNames[0]);
}

TEST(CmagTargetsFilesListFileParseTest, givenMultipleConfigsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    [
        "project_Debug.cmag-targets-list",
        "project_Release.cmag-targets-list",
        "project_CustomConf.cmag-targets-list"
    ]
    )DELIMETER";
    std::vector<std::string> fileNames{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFilesListFile(json, fileNames).status);
    ASSERT_EQ(3u, fileNames.size());
    EXPECT_EQ("project_Debug.cmag-targets-list", fileNames[0]);
    EXPECT_EQ("project_Release.cmag-targets-list", fileNames[1]);
    EXPECT_EQ("project_CustomConf.cmag-targets-list", fileNames[2]);
}

TEST(CmagGlobalsFileParseTest, givenEmptyGlobalsThenReturnError) {
    const char *json = R"DELIMETER(
    {}
    )DELIMETER";
    CmagGlobals globals{};
    ASSERT_EQ(ParseResultStatus::MissingField, CmagJsonParser::parseGlobalsFile(json, globals).status);
    EXPECT_FALSE(globals.darkMode);
}

TEST(CmagGlobalsFileParseTest, givenAllFieldsSpecifiedThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "darkMode": true,
        "selectedConfig": "Z",
        "cmagVersion": "0.1.15",
        "cmakeVersion": "B",
        "cmakeProjectName": "C",
        "cmagProjectName": "D",
        "sourceDir": "E",
        "buildDir": "F",
        "generator": "G",
        "compilerId": "H",
        "compilerVersion": "I",
        "os": "J",
        "useFolders": "K",
        "listDirs": { "K": [ "L" ], "L": [] },
        "browser": {
            "needsLayout": true,
            "autoSaveEnabled": true,
            "cameraX": 888.0,
            "cameraY": 777.0,
            "cameraScale": 999.0,
            "displayedDependencyType": 4,
            "selectedTabIndex": 3,
            "selectedTargetName": "ZERO"
        }
    }
    )DELIMETER";
    CmagGlobals globals{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseGlobalsFile(json, globals).status);
    EXPECT_TRUE(globals.darkMode);
    EXPECT_STREQ(globals.selectedConfig.c_str(), "Z");
    EXPECT_STREQ(globals.cmagVersion.toString().c_str(), "0.1.15");
    EXPECT_STREQ(globals.cmakeVersion.c_str(), "B");
    EXPECT_STREQ(globals.cmakeProjectName.c_str(), "C");
    EXPECT_STREQ(globals.cmagProjectName.c_str(), "D");
    EXPECT_STREQ(globals.sourceDir.c_str(), "E");
    EXPECT_STREQ(globals.buildDir.c_str(), "F");
    EXPECT_STREQ(globals.generator.c_str(), "G");
    EXPECT_STREQ(globals.compilerId.c_str(), "H");
    EXPECT_STREQ(globals.compilerVersion.c_str(), "I");
    EXPECT_STREQ(globals.os.c_str(), "J");
    EXPECT_STREQ(globals.useFolders.c_str(), "K");
    EXPECT_TRUE(globals.browser.needsLayout);
    EXPECT_TRUE(globals.browser.needsLayout);
    EXPECT_EQ(888.0, globals.browser.cameraX);
    EXPECT_EQ(777.0, globals.browser.cameraY);
    EXPECT_EQ(999.0, globals.browser.cameraScale);
    EXPECT_EQ(CmagDependencyType::Additional, globals.browser.displayedDependencyType);
    EXPECT_EQ(3, globals.browser.selectedTabIndex);
    EXPECT_STREQ("ZERO", globals.browser.selectedTargetName.c_str());

    ASSERT_EQ(2u, globals.listDirs.size());
    EXPECT_STREQ("K", globals.listDirs[0].name.c_str());
    EXPECT_EQ(1u, globals.listDirs[0].childIndices.size());
    EXPECT_EQ(1u, globals.listDirs[0].childIndices[0]);
    EXPECT_STREQ("L", globals.listDirs[1].name.c_str());
    EXPECT_EQ(0u, globals.listDirs[1].childIndices.size());
}

TEST(CmagTargetsFileParseTest, givenNoTargetsThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
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
    ASSERT_EQ(ParseResultStatus::MissingField, CmagJsonParser::parseTargetsFile(json, targets).status);
}

TEST(CmagTargetsFileParseTest, givenTargetWithNoPropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget" : {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "non_genexable": {},
                    "genexable": {},
                    "genexable_evaled": {}
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
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

    EXPECT_STREQ("a", target.listDirName.c_str());
}

TEST(CmagTargetsFileParseTest, givenTargetWithNonGenexablePropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget": {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "non_genexable": {
                        "one": "1",
                        "two": "2"
                    },
                    "genexable": {},
                    "genexable_evaled": {}
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
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
                    "non_genexable": {
                        "one": "1",
                        "two": "2"
                    },
                    "genexable": {},
                    "genexable_evaled": {}
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        },
        "myTarget2" : {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "non_genexable": {
                        "three": "3",
                        "zfour": "4"
                    },
                    "genexable": {},
                    "genexable_evaled": {}
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
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

TEST(CmagTargetsFileParseTest, givenTargetWithGenexablePropertesThenParseCorrectly) {
    const char *json = R"DELIMETER(
    {
        "myTarget": {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "non_genexable": {},
                    "genexable": {
                        "one": "1",
                        "two": "2"
                    },
                    "genexable_evaled": {
                        "one": "1",
                        "two": "2"
                    }
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
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

TEST(CmagTargetsFileParseTest, givenTargetWithLinkOnlyGenexPropertesThenParseAndStripIt) {
    const char *json = R"DELIMETER(
    {
        "myTarget": {
            "type": "EXECUTABLE",
            "configs": {
                "Debug": {
                    "non_genexable": {},
                    "genexable": {
                        "INTERFACE_LINK_LIBRARIES": "Lib1;Lib2;$<LINK_ONLY:Lib3>",
                        "LINK_LIBRARIES": "$<LINK_ONLY:Lib1>;Lib2;Lib3"
                    },
                    "genexable_evaled": {
                        "INTERFACE_LINK_LIBRARIES": "Lib1;Lib2;Lib3",
                        "LINK_LIBRARIES": "Lib1;Lib2;Lib3"
                    }
                }
            },
            "listDir" : "a",
            "isImported": false,
            "aliases": []
        }
    }
    )DELIMETER";
    std::vector<CmagTarget> targets{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseTargetsFile(json, targets).status);
    ASSERT_EQ(1u, targets.size());

    const CmagTarget &target = targets[0];
    EXPECT_STREQ("myTarget", target.name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, target.type);

    CmagTargetConfig expectedProperties = {
        "Debug",
        {
            {"INTERFACE_LINK_LIBRARIES", "Lib1;Lib2"},
            {"LINK_LIBRARIES", "Lib2;Lib3"},
        },
    };
    ASSERT_EQ(1u, target.configs.size());
    compareTargetProperties(expectedProperties, target.configs[0]);
}

TEST(CmagAliasesFileParseTest, givenEmptyAliasesListThenReturnEmptyList) {
    const char *json = R"DELIMETER(
    {}
    )DELIMETER";
    std::vector<std::pair<std::string, std::string>> aliases{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseAliasesFile(json, aliases).status);
    ASSERT_EQ(0u, aliases.size());
}

TEST(CmagAliasesFileParseTest, givenNonEmptyAliasesListThenReadThemCorrectly) {
    const char *json = R"DELIMETER(
    {
        "ABC": "abc",
        "json::json": "json"
    }
    )DELIMETER";
    std::vector<std::pair<std::string, std::string>> aliases{};
    ASSERT_EQ(ParseResultStatus::Success, CmagJsonParser::parseAliasesFile(json, aliases).status);
    ASSERT_EQ(2u, aliases.size());

    EXPECT_STREQ("ABC", aliases[0].first.c_str());
    EXPECT_STREQ("abc", aliases[0].second.c_str());

    EXPECT_STREQ("json::json", aliases[1].first.c_str());
    EXPECT_STREQ("json", aliases[1].second.c_str());
}
