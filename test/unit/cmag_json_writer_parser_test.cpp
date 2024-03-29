#include "cmag_core/parse/cmag_json_parser.h"
#include "cmag_core/parse/cmag_json_writer.h"

#include <gtest/gtest.h>

struct CmagWriterParserTest : ::testing::Test {
    CmagProject project = {};

    void SetUp() override {
        initProject(project);
    }

    static void initProject(CmagProject &project) {
        project.getGlobals().cmagVersion = ::cmagVersion;
    }

    static void verify(const CmagProject &initialProject) {
        std::ostringstream jsonStream;
        CmagJsonWriter::writeProject(initialProject, jsonStream);
        std::string json = jsonStream.str();

        CmagProject derivedProject{};
        const auto parseResult = CmagJsonParser::parseProject(json, derivedProject);
        if (parseResult.status != ParseResultStatus::Success && parseResult.status != ParseResultStatus::DataDerivationFailed) {
            // We can allow data derivation failure, since it's not important here
            ASSERT_EQ(ParseResultStatus::Success, parseResult.status);
        }

        compareProjects(initialProject, derivedProject);
    }

    static void compareProjects(const CmagProject &exp, const CmagProject &act) {
        // This function naively assumes that the order of configs and/or properties will not change.
        // While this is completely uninmportant for json parsing, maintaining stable order greatly
        // eases testing. To achieve this, we have to put everything alphabetically, e.g. "Debug" config
        // has to be before "Release" config and "propertyL" has to be before "propertyM".

        EXPECT_EQ(exp.getConfigs(), act.getConfigs());

        const auto &expGlobals = exp.getGlobals();
        const auto &actGlobals = act.getGlobals();
        EXPECT_EQ(expGlobals.darkMode, actGlobals.darkMode);
        EXPECT_EQ(expGlobals.selectedConfig, actGlobals.selectedConfig);
        EXPECT_EQ(expGlobals.cmagVersion, actGlobals.cmagVersion);
        EXPECT_EQ(expGlobals.cmakeVersion, actGlobals.cmakeVersion);
        EXPECT_EQ(expGlobals.cmakeProjectName, actGlobals.cmakeProjectName);
        EXPECT_EQ(expGlobals.cmagProjectName, actGlobals.cmagProjectName);
        EXPECT_EQ(expGlobals.sourceDir, actGlobals.sourceDir);
        EXPECT_EQ(expGlobals.buildDir, actGlobals.buildDir);
        EXPECT_EQ(expGlobals.generator, actGlobals.generator);
        EXPECT_EQ(expGlobals.compilerId, actGlobals.compilerId);
        EXPECT_EQ(expGlobals.compilerVersion, actGlobals.compilerVersion);
        EXPECT_EQ(expGlobals.os, actGlobals.os);
        EXPECT_EQ(expGlobals.useFolders, actGlobals.useFolders);

        EXPECT_EQ(expGlobals.browser.needsLayout, actGlobals.browser.needsLayout);
        EXPECT_EQ(expGlobals.browser.autoSaveEnabled, actGlobals.browser.autoSaveEnabled);
        EXPECT_EQ(expGlobals.browser.cameraX, actGlobals.browser.cameraX);
        EXPECT_EQ(expGlobals.browser.cameraY, actGlobals.browser.cameraY);
        EXPECT_EQ(expGlobals.browser.cameraScale, actGlobals.browser.cameraScale);
        EXPECT_EQ(expGlobals.browser.displayedDependencyType, actGlobals.browser.displayedDependencyType);
        EXPECT_EQ(expGlobals.browser.selectedTabIndex, actGlobals.browser.selectedTabIndex);
        EXPECT_EQ(expGlobals.browser.selectedTargetName, actGlobals.browser.selectedTargetName);

        ASSERT_EQ(expGlobals.listDirs.size(), actGlobals.listDirs.size());
        for (size_t i = 0; i < expGlobals.listDirs.size(); i++) {
            const auto &expListDir = expGlobals.listDirs[i];
            const auto &actListDir = actGlobals.listDirs[i];
            EXPECT_EQ(expListDir.name, actListDir.name);
            ASSERT_EQ(expListDir.childIndices.size(), actListDir.childIndices.size());
            for (size_t j = 0; j < expListDir.childIndices.size(); j++) {
                const auto &expChildIndex = expListDir.childIndices[j];
                const auto &actChildIndex = actListDir.childIndices[j];
                EXPECT_EQ(expChildIndex, actChildIndex);
            }
        }

        const auto &expTargets = exp.getTargets();
        const auto &actTargets = act.getTargets();
        ASSERT_EQ(expTargets.size(), actTargets.size());
        for (size_t i = 0; i < expTargets.size(); i++) {
            const auto &expTarget = expTargets[i];
            const auto &actTarget = actTargets[i];

            EXPECT_EQ(expTarget.name, actTarget.name);
            EXPECT_EQ(expTarget.type, actTarget.type);
            EXPECT_EQ(expTarget.listDirName, actTarget.listDirName);
            EXPECT_EQ(expTarget.isImported, actTarget.isImported);

            ASSERT_EQ(expTarget.configs.size(), actTarget.configs.size());
            for (size_t j = 0; j < expTarget.configs.size(); j++) {
                const auto &expPropertiesForConfig = expTarget.configs[j];
                const auto &actPropertiesForConfig = actTarget.configs[j];

                EXPECT_EQ(expPropertiesForConfig.name, actPropertiesForConfig.name);
                ASSERT_EQ(expPropertiesForConfig.properties.size(), actPropertiesForConfig.properties.size());
                for (size_t k = 0; k < expPropertiesForConfig.properties.size(); k++) {
                    const auto &expProperty = expPropertiesForConfig.properties[k];
                    const auto &actProperty = actPropertiesForConfig.properties[k];
                    EXPECT_EQ(expProperty.name, actProperty.name);
                    EXPECT_EQ(expProperty.value, actProperty.value);
                }
            }

            const auto &expGraphical = expTarget.graphical;
            const auto &actGraphical = actTarget.graphical;
            EXPECT_EQ(expGraphical.x, actGraphical.x);
            EXPECT_EQ(expGraphical.y, actGraphical.y);
            EXPECT_EQ(expGraphical.hideConnections, actGraphical.hideConnections);

            ASSERT_EQ(expTarget.aliases.size(), actTarget.aliases.size());
            for (size_t j = 0; j < expTarget.aliases.size(); j++) {
                const auto &expAlias = expTarget.aliases[j];
                const auto &actAlias = actTarget.aliases[j];
                EXPECT_EQ(expAlias, actAlias);
            }
        }
    }
};

TEST_F(CmagWriterParserTest, givenProjectWithNoTargetsThenWriteAndReadCorrectly) {
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithoutPropertiesThenWriteAndReadCorrectly) {
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetAliasesThenWriteAndReadCorrectly) {
    auto addTarget = [&](const std::string &targetName, const std::vector<std::string> &aliases) {
        CmagTarget target{
            targetName,
            CmagTargetType::Executable,
            {
                {"Debug", {}},
            },
        };
        target.aliases = aliases;

        project.addTarget(std::move(target));
    };

    addTarget("target1", {"A", "B", "C"});
    addTarget("target2", {"D", "E"});
    addTarget("target3", {});
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithGraphicalDataThenWriteAndReadCorrectly) {
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {
            12.f,
            25.3f,
            true,
        },
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithPropertiesThenWriteAndReadCorrectly) {
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"prop1", "one"},
                    {"prop2", "two"},
                },
            },
        },
        {},
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithMultipleConfigsThenWriteAndReadCorrectly) {
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"prop1", "one"},
                    {"prop2", "two"},
                },
            },
            {
                "Release",
                {
                    {"prop1", "one"},
                    {"prop2", "two"},
                },
            },
        },
        {},
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithMultipleConfigsAndDifferingPropertiesThenWriteAndReadCorrectly) {
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"prop1", "oneDebug"},
                    {"prop2", "two"},
                },
            },
            {
                "Release",
                {
                    {"prop1", "oneRelease"},
                    {"prop2", "two"},
                    {"prop3", "three"},
                },
            },
        },
        {},
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithImportedTargetWithoutPropertiesThenWriteAndReadCorrectly) {
    auto addTarget = [&](const char *targetName, bool isImported) {
        CmagTarget target{
            targetName,
            CmagTargetType::Executable,
            {
                {"Debug", {}},
            },
        };
        target.isImported = isImported;
        project.addTarget(std::move(target));
    };

    addTarget("target1", false);
    addTarget("target2", true);
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithVariousTargetTypesThenWriteAndReadCorrectly) {
    CmagTargetType types[] = {
        CmagTargetType::StaticLibrary,
        CmagTargetType::ModuleLibrary,
        CmagTargetType::SharedLibrary,
        CmagTargetType::ObjectLibrary,
        CmagTargetType::InterfaceLibrary,
        CmagTargetType::Utility,
        CmagTargetType::Executable,
    };
    for (CmagTargetType type : types) {
        CmagProject cmagProject;
        initProject(cmagProject);
        cmagProject.addTarget(CmagTarget{
            "myTarget",
            type,
            {
                {
                    "Debug",
                    {
                        {"prop1", "oneDebug"},
                        {"prop2", "two"},
                    },
                },
            },
            {},
        });
        verify(cmagProject);
    }
}

TEST_F(CmagWriterParserTest, givenProjectWithSetGlobalsThenWriteAndReadCorrectly) {
    CmagGlobals &globals = project.getGlobals();
    globals.darkMode = true;
    globals.selectedConfig = "Debug",
    globals.cmagVersion = ::cmagVersion;
    globals.cmakeVersion = "3.5.9";
    globals.cmakeProjectName = "myProject";
    globals.cmagProjectName = "myCmagProject";
    globals.sourceDir = "src";
    globals.buildDir = "src/build";
    globals.generator = "Karate Ninja";
    globals.compilerId = "1.0.0.0.0";
    globals.compilerVersion = "GNU Clang";
    globals.os = "Serenity";
    globals.useFolders = "ON";

    globals.browser.needsLayout = true;
    globals.browser.autoSaveEnabled = true;
    globals.browser.cameraX = 123.2f;
    globals.browser.cameraY = 321.2f;
    globals.browser.cameraScale = 0.1f;
    globals.browser.displayedDependencyType = CmagDependencyType::Build | CmagDependencyType::Interface;
    globals.browser.selectedTabIndex = 3;
    globals.browser.selectedTargetName = "ALL";

    globals.listDirs = {
        CmagListDir{"a", {1, 2}},
        CmagListDir{"b", {}},
        CmagListDir{"c", {3}},
        CmagListDir{"d", {}},
    };
    verify(project);
}
