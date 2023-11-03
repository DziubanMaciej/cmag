#include "cmag_lib/core/cmag.h"
#include "cmag_lib/parse/cmag_json_parser.h"
#include "cmag_lib/utils/file_utils.h"
#include "test/os/cmake_generator_db.h"
#include "test/os/fixtures.h"

struct WhiteboxCmag : Cmag {
    using Cmag::Cmag;
    using Cmag::project;
};

struct CmagTest : CmagOsTest, testing::WithParamInterface<CMakeGenerator> {
    static std::vector<std::string> constructCmakeArgs(const ProjectInfo &workspace) {
        return {
            "cmake",
            "-S",
            workspace.sourcePath,
            "-B",
            workspace.buildPath,
            "-G",
            GetParam().name,
            "--graphviz",
            workspace.graphvizPath,
        };
    }

    static std::string constructParamName(const testing::TestParamInfo<CMakeGenerator> &param) {
        return param.param.gtestName;
    }

    static void verifyConfigNaming(const CmagTarget &target) {
        if (GetParam().isMultiConfig) {
            EXPECT_LE(2u, target.configs.size());
            EXPECT_STREQ("Debug", target.configs[0].name.c_str());
            EXPECT_STREQ("Release", target.configs[1].name.c_str());
        } else {
            EXPECT_EQ(1u, target.configs.size());
            EXPECT_STREQ("Default", target.configs[0].name.c_str());
        }
    }

    static void verifyProperty(const CmagTargetConfig &config, const char *name, const char *expectedValue) {
        for (const CmagTargetProperty &prop : config.properties) {
            if (prop.name == name) {
                EXPECT_STREQ(expectedValue, prop.value.c_str());
                return;
            }
        }
        EXPECT_TRUE(false) << "property " << name << " not found";
    }

    static void verifyPropertyForEachConfig(const CmagTarget &target, const char *name, const char *expectedValue) {
        for (const CmagTargetConfig &config : target.configs) {
            verifyProperty(config, name, expectedValue);
        }
    }

    static void verifyNoProperty(const CmagTargetConfig &config, const char *name) {
        for (const CmagTargetProperty &prop : config.properties) {
            EXPECT_STRNE(name, prop.name.c_str());
        }
    }

    static std::vector<CmagTarget> getSortedTargets(const CmagProject &project) {
        std::vector<CmagTarget> targets = project.getTargets();
        std::sort(targets.begin(), targets.end(), [](const CmagTarget &left, const CmagTarget &right) {
            return left.name < right.name;
        });
        return targets;
    }
};

TEST_P(CmagTest, givenSimpleProjectWithCustomPropertiesThenProcessItCorrectly) {
    ProjectInfo workspace = prepareProject("simple");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};

    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace),
                                                          "MY_CUSTOM_PROP1;MY_CUSTOM_PROP2"));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    ASSERT_EQ(1u, cmag.project.getTargets().size());
    const CmagTarget &target = cmag.project.getTargets()[0];
    EXPECT_STREQ("Exe", target.name.c_str());
    verifyConfigNaming(target);
    for (const CmagTargetConfig &config : target.configs) {
        verifyProperty(config, "MY_CUSTOM_PROP1", "customValue1");
        verifyProperty(config, "MY_CUSTOM_PROP2", "customValue2");
        verifyNoProperty(config, "MY_CUSTOM_PROP3");
    }
}

TEST_P(CmagTest, givenProjectWrittenToFileThenItCanBeParsedBack) {
    ProjectInfo workspace = prepareProject("simple");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"test_proj"};

    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace),
                                                          "MY_CUSTOM_PROP1;MY_CUSTOM_PROP2"));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
        ASSERT_EQ(CmagResult::Success, cmag.writeProjectToFile(workspace.buildPath));
    }

    std::optional<std::string> projectJson = readFile(workspace.buildPath / "test_proj.cmag-project");
    ASSERT_TRUE(projectJson.has_value());

    CmagProject parsedProject = {};
    ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(projectJson.value(), parsedProject));

    ASSERT_EQ(1u, parsedProject.getTargets().size());
    const CmagTarget &target = parsedProject.getTargets()[0];
    EXPECT_STREQ("Exe", target.name.c_str());
    for (const CmagTargetConfig &config : target.configs) {
        verifyProperty(config, "MY_CUSTOM_PROP1", "customValue1");
        verifyProperty(config, "MY_CUSTOM_PROP2", "customValue2");
        verifyNoProperty(config, "MY_CUSTOM_PROP3");
    }
}

TEST_P(CmagTest, givenProjectWithAllTargetTypesThenAllTargetsAreDetectedCorrectly) {
    ProjectInfo workspace = prepareProject("all_types");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};

    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace),
                                                          "MY_CUSTOM_PROP1;MY_CUSTOM_PROP2"));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    auto targets = getSortedTargets(cmag.project);
    ASSERT_EQ(6u, targets.size());

    EXPECT_STREQ("Executable", targets[0].name.c_str());
    EXPECT_EQ(CmagTargetType::Executable, targets[0].type);
    verifyPropertyForEachConfig(targets[0], "SOURCES", "file.cpp");
    verifyPropertyForEachConfig(targets[0], "LINK_LIBRARIES", "StaticLib;SharedLib;ObjectLib;InterfaceLib");

    EXPECT_STREQ("InterfaceLib", targets[1].name.c_str());
    EXPECT_EQ(CmagTargetType::InterfaceLibrary, targets[1].type);
    verifyPropertyForEachConfig(targets[1], "SOURCES", "");
    verifyPropertyForEachConfig(targets[1], "INTERFACE_LINK_LIBRARIES", "SharedLib");

    EXPECT_STREQ("ModuleLib", targets[2].name.c_str());
    EXPECT_EQ(CmagTargetType::ModuleLibrary, targets[2].type);
    verifyPropertyForEachConfig(targets[2], "SOURCES", "file.cpp");

    EXPECT_STREQ("ObjectLib", targets[3].name.c_str());
    EXPECT_EQ(CmagTargetType::ObjectLibrary, targets[3].type);
    verifyPropertyForEachConfig(targets[3], "SOURCES", "file.cpp");

    EXPECT_STREQ("SharedLib", targets[4].name.c_str());
    EXPECT_EQ(CmagTargetType::SharedLibrary, targets[4].type);
    verifyPropertyForEachConfig(targets[4], "MANUALLY_ADDED_DEPENDENCIES", "StaticLib");
    verifyPropertyForEachConfig(targets[4], "SOURCES", "file.cpp");

    EXPECT_STREQ("StaticLib", targets[5].name.c_str());
    EXPECT_EQ(CmagTargetType::StaticLibrary, targets[5].type);
    verifyPropertyForEachConfig(targets[5], "SOURCES", "file.cpp");
}

TEST_P(CmagTest, givenProjectWithTargetsDefinedInSubdirectoriesThenAllTargetAreDetectedCorrectly) {
    ProjectInfo workspace = prepareProject("with_subdirs");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};
    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace), ""));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    ASSERT_EQ(6u, cmag.project.getTargets().size());

    auto targets = getSortedTargets(cmag.project);
    ASSERT_EQ(6u, targets.size());

    {
        const CmagTarget &target = targets[0];
        EXPECT_STREQ("Executable", target.name.c_str());
        EXPECT_EQ(CmagTargetType::Executable, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "LibA;LibB");
    }
    {
        const CmagTarget &target = targets[1];
        EXPECT_STREQ("LibA", target.name.c_str());
        EXPECT_EQ(CmagTargetType::StaticLibrary, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "");
    }
    {
        const CmagTarget &target = targets[2];
        EXPECT_STREQ("LibB", target.name.c_str());
        EXPECT_EQ(CmagTargetType::StaticLibrary, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "LibC;LibD");
        // verifyPropertyForEachConfig(target, "INTERFACE_LINK_LIBRARIES", "LibC;LibE"); // TODO remove $<LINK_ONLY:...>
        verifyPropertyForEachConfig(target, "INCLUDE_DIRECTORIES", "/DirC;/DirD");
        verifyPropertyForEachConfig(target, "INTERFACE_INCLUDE_DIRECTORIES", "/DirC;/DirE");
    }
    {
        const CmagTarget &target = targets[3];
        EXPECT_STREQ("LibC", target.name.c_str());
        EXPECT_EQ(CmagTargetType::StaticLibrary, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INTERFACE_LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INCLUDE_DIRECTORIES", "/DirC");
        verifyPropertyForEachConfig(target, "INTERFACE_INCLUDE_DIRECTORIES", "/DirC");
    }
    {
        const CmagTarget &target = targets[4];
        EXPECT_STREQ("LibD", target.name.c_str());
        EXPECT_EQ(CmagTargetType::StaticLibrary, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INTERFACE_LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INCLUDE_DIRECTORIES", "/DirD");
        verifyPropertyForEachConfig(target, "INTERFACE_INCLUDE_DIRECTORIES", "/DirD");
    }
    {
        const CmagTarget &target = targets[5];
        EXPECT_STREQ("LibE", target.name.c_str());
        EXPECT_EQ(CmagTargetType::StaticLibrary, target.type);
        verifyPropertyForEachConfig(target, "LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INTERFACE_LINK_LIBRARIES", "");
        verifyPropertyForEachConfig(target, "INCLUDE_DIRECTORIES", "/DirE");
        verifyPropertyForEachConfig(target, "INTERFACE_INCLUDE_DIRECTORIES", "/DirE");
    }
}

TEST_P(CmagTest, givenGeneratorExpressionsInPropertiesThenResolveThemToActualValues) {
    ProjectInfo workspace = prepareProject("genex");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};
    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success,
                  cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace), "CUSTOM_PROP"));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    ASSERT_EQ(1u, cmag.project.getTargets().size());

    const CmagTarget &target = cmag.project.getTargets()[0];
    for (const CmagTargetConfig &config : target.configs) {
        const bool isDebug = config.name == "Debug";
        const char *letter = isDebug ? "D" : "R";
        const char *letterPath = isDebug ? "/D" : "/R";
        const char *letterDef = isDebug ? "VALUE=D" : "VALUE=R";

        verifyProperty(config, "LINK_DIRECTORIES", letterPath);
        verifyProperty(config, "LINK_LIBRARIES", letter);
        verifyProperty(config, "COMPILE_DEFINITIONS", letterDef);
        verifyProperty(config, "COMPILE_OPTIONS", letter);

        verifyProperty(config, "INTERFACE_LINK_DIRECTORIES", letterPath);
        verifyProperty(config, "INTERFACE_LINK_LIBRARIES", letter);
        verifyProperty(config, "INTERFACE_INCLUDE_DIRECTORIES", letterPath);
        verifyProperty(config, "INTERFACE_COMPILE_OPTIONS", letter);

        verifyProperty(config, "CUSTOM_PROP", "$<IF:$<CONFIG:Debug>,D,R>");
    }
}

TEST_P(CmagTest, whenGeneratingPositionsForGraphThenPositionsAreNonZero) {
    ProjectInfo workspace = prepareProject("simple");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};
    {
        RaiiStdoutCapture capture{};
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace), ""));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
        ASSERT_EQ(CmagResult::Success,
                  cmag.generateGraphPositionsForProject(workspace.buildPath, workspace.graphvizPath));
    }

    ASSERT_EQ(1u, cmag.project.getTargets().size());
    const CmagTarget &target = cmag.project.getTargets()[0];
    EXPECT_NE(0u, target.graphical.x);
    EXPECT_NE(0u, target.graphical.y);
}

TEST_P(CmagTest, givenMultiConfigGeneratorCustomConfigNamesSpecifiedThenProcessThemCorrectly) {
    if (!GetParam().isMultiConfig) {
        GTEST_SKIP();
    }

    ProjectInfo workspace = prepareProject("simple");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};
    {
        RaiiStdoutCapture capture{};
        auto cmakeArgs = constructCmakeArgs(workspace);
        cmakeArgs.emplace_back("-DCMAKE_CONFIGURATION_TYPES=Elmo;CookieMonster");
        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, cmakeArgs, ""));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    ASSERT_EQ(1u, cmag.project.getTargets().size());
    const CmagTarget &target = cmag.project.getTargets()[0];
    EXPECT_STREQ("Exe", target.name.c_str());
    ASSERT_EQ(2u, target.configs.size());
    {
        const CmagTargetConfig &config = target.configs[0];
        EXPECT_STREQ("Elmo", config.name.c_str());
        verifyProperty(config, "COMPILE_OPTIONS", "OptionElmo");
    }
    {
        const CmagTargetConfig &config = target.configs[1];
        EXPECT_STREQ("CookieMonster", config.name.c_str());
        verifyProperty(config, "COMPILE_OPTIONS", "OptionCookieMonster");
    }
}

TEST_P(CmagTest, givenSingleConfigGeneratorCustomConfigNameSpecifiedThenProcessItCorrectly) {
    if (GetParam().isMultiConfig) {
        GTEST_SKIP();
    }

    ProjectInfo workspace = prepareProject("simple");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};
    {
        RaiiStdoutCapture capture{};
        auto cmakeArgs = constructCmakeArgs(workspace);
        cmakeArgs.emplace_back("-DCMAKE_BUILD_TYPE=Elmo");

        ASSERT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, cmakeArgs, ""));
        ASSERT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));
    }

    ASSERT_EQ(1u, cmag.project.getTargets().size());
    const CmagTarget &target = cmag.project.getTargets()[0];
    EXPECT_STREQ("Exe", target.name.c_str());
    ASSERT_EQ(1u, target.configs.size());
    {
        const CmagTargetConfig &config = target.configs[0];
        EXPECT_STREQ("Elmo", config.name.c_str());
        verifyProperty(config, "COMPILE_OPTIONS", "OptionElmo");
    }
}

INSTANTIATE_TEST_SUITE_P(, CmagTest, ::testing::ValuesIn(CmakeGeneratorDb::instance().generators),
                         CmagTest::constructParamName);