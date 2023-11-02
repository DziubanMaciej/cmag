#include "cmag_lib/core/cmag.h"
#include "test/os/cmake_generator_db.h"
#include "test/os/fixtures.h"

struct WhiteboxCmag : Cmag {
    using Cmag::Cmag;
    using Cmag::project;
};

struct CmagTest : CmagOsTest, testing::WithParamInterface<CMakeGenerator> {
    void SetUp() override {
        CmagOsTest::SetUp();
        testing::internal::CaptureStdout();
    }

    void TearDown() override {
        CmagOsTest::TearDown();
        testing::internal::GetCapturedStdout();
    }

    static std::vector<std::string> constructCmakeArgs(const ProjectInfo &workspace) {
        return {
            "cmake",
            "-S",
            workspace.sourcePath,
            "-B",
            workspace.buildPath,
            "-G",
            GetParam().name,
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

    std::vector<CmagTarget> getSortedTargets(const CmagProject &project) {
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

    EXPECT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace), "MY_CUSTOM_PROP1;MY_CUSTOM_PROP2"));
    EXPECT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));

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

TEST_P(CmagTest, givenProjectWithAllTargetTypesThenAllTargetsAreDetectedCorrectly) {
    ProjectInfo workspace = prepareProject("all_types");
    ASSERT_TRUE(workspace.valid);

    WhiteboxCmag cmag{"project"};

    EXPECT_EQ(CmagResult::Success, cmag.generateCmake(workspace.sourcePath, constructCmakeArgs(workspace), "MY_CUSTOM_PROP1;MY_CUSTOM_PROP2"));
    EXPECT_EQ(CmagResult::Success, cmag.readCmagProjectFromGeneration(workspace.buildPath));

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

// givenProjectWithAllTargetTypesThenAllTargetsAreDetectedCorrectly
// givenProjectWithTargetsDefinedInSubdirectoriesThenAllTargetAreDetectedCorrectly
// givenGeneratorExpressionsInPropertiesThenResolveThemToActualValues
// whenGeneratingPositionsForGraphThenPositionsAreNonZero
// givenProjectWrittenToFileThenItCanBeParsedBack
// givenCustomConfigNamesSpecifiedThenProcessThemCorrectly

INSTANTIATE_TEST_SUITE_P(, CmagTest, ::testing::ValuesIn(CmakeGeneratorDb::instance().generators), CmagTest::constructParamName);


