#include "cmag_core/core/cmag_project.h"
#include "cmag_core/core/cmake_generator.h"

#include <gtest/gtest.h>

TEST(CmagProjectTest, givenTargetsWithDifferentNamesAreAddedThenAddAllOfThem) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target1",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_TRUE(project.addTarget(CmagTarget{target2}));
    ASSERT_EQ(2u, project.getTargets().size());
    EXPECT_EQ(target1.name, project.getTargets()[0].name);
    EXPECT_EQ(target2.name, project.getTargets()[1].name);
}

TEST(CmagProjectTest, givenTargetsWithSameNamesAndDifferentConfigsAreAddedThenAddAllOfThem) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_TRUE(project.addTarget(CmagTarget{target2}));
    ASSERT_EQ(1u, project.getTargets().size());
    EXPECT_EQ(target1.name, project.getTargets()[0].name);
    EXPECT_EQ(3u, project.getTargets()[0].configs.size());
}

TEST(CmagProjectTest, givenTargetsWithSameNamesAndConflictingConfigsAreAddedThenReturnError) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
            {"Debug", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_FALSE(project.addTarget(CmagTarget{target2}));
}

TEST(CmagProjectTest, givenTargetsWithSameNamesAndConflictingTypesAreAddedThenReturnError) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::StaticLibrary,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_FALSE(project.addTarget(CmagTarget{target2}));
}

TEST(CmagProjectTest, givenTargetsWithVariousConfigsAreAddedThenCollectAllConfigs) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target1",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
        {},
    };

    CmagTarget target3 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
            {"MinSizeRel", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_TRUE(project.addTarget(CmagTarget{target2}));
    EXPECT_TRUE(project.addTarget(CmagTarget{target3}));
    ASSERT_EQ(4u, project.getConfigs().size());
    EXPECT_STREQ("Debug", project.getConfigs()[0].c_str());
    EXPECT_STREQ("Release", project.getConfigs()[1].c_str());
    EXPECT_STREQ("ReleaseWithDebInfo", project.getConfigs()[2].c_str());
    EXPECT_STREQ("MinSizeRel", project.getConfigs()[3].c_str());
}

TEST(CmagTargetTest, givenConfigExistsWhenGetOrCreateConfigIsCalledThenReturnExistingConfig) {
    CmagTarget target = {
        "target",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"key1", "value1"},
                    {"key2", "value2"},
                },
            },
            {
                "Release",
                {
                    {"key1", "value1"},
                    {"key2", "value2"},
                },
            },
        },
        {},
    };

    {
        CmagTargetConfig &expectedConfig = target.configs[0];
        CmagTargetConfig &queriedConfig = target.getOrCreateConfig("Debug");
        EXPECT_EQ(&expectedConfig, &queriedConfig);
        EXPECT_EQ(2u, target.configs.size());
    }
    {
        CmagTargetConfig &expectedConfig = target.configs[1];
        CmagTargetConfig &queriedConfig = target.getOrCreateConfig("Release");
        EXPECT_EQ(&expectedConfig, &queriedConfig);
        EXPECT_EQ(2u, target.configs.size());
    }
}

TEST(CmagTargetTest, givenConfigDoesNotExistWhenGetOrCreateConfigIsCalledThenCreateAndReturnNewConfig) {
    CmagTarget target = {
        "target",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"key1", "value1"},
                    {"key2", "value2"},
                },
            },
            {
                "Release",
                {
                    {"key1", "value1"},
                    {"key2", "value2"},
                },
            },
        },
        {},
    };

    CmagTargetConfig &queriedConfig = target.getOrCreateConfig("RelWithDebInfo");
    EXPECT_EQ(&target.configs[2], &queriedConfig);
    EXPECT_EQ(3u, target.configs.size());
    EXPECT_STREQ("RelWithDebInfo", queriedConfig.name.c_str());
    EXPECT_EQ(0u, queriedConfig.properties.size());
}

TEST(CmagProjectTest, givenAliasesWhenAddingTargetAliasesThenFillAliasesVectorCorrectly) {
    CmagProject project = {};

    CmagTarget target1 = {
        "target1",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    CmagTarget target2 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
        {},
    };

    EXPECT_TRUE(project.addTarget(CmagTarget{target1}));
    EXPECT_TRUE(project.addTarget(CmagTarget{target2}));
    EXPECT_TRUE(project.addTargetAlias("A", "target1"));
    EXPECT_TRUE(project.addTargetAlias("B", "target1"));
    EXPECT_TRUE(project.addTargetAlias("C", "target2"));
    EXPECT_TRUE(project.addTargetAlias("D", "target2"));
    EXPECT_FALSE(project.addTargetAlias("E", "target3"));
    EXPECT_FALSE(project.addTargetAlias("F", "target3"));

    ASSERT_EQ(2u, project.getTargets().size());
    {
        const CmagTarget &target = project.getTargets()[0];
        EXPECT_EQ(target1.name, target.name);
        EXPECT_EQ((std::vector<std::string>{"A", "B"}), target.aliases);
    }
    {
        const CmagTarget &target = project.getTargets()[1];
        EXPECT_EQ(target2.name, target.name);
        EXPECT_EQ((std::vector<std::string>{"C", "D"}), target.aliases);
    }
}

struct CmagProjectDeriveTest : ::testing::Test {
    struct CmagProjectWhitebox : CmagProject {
        using CmagProject::CmagProject;

        bool deriveData() {
            // Call two times to ensure everything is cleaned after the first run.
            return CmagProject::deriveData() && CmagProject::deriveData();
        }
    };

    CmagProjectWhitebox project = {};
};

TEST_F(CmagProjectDeriveTest, givenTargetsWithDependenciesWhenDerivingDataThenDependenciesAreSavedToTheVectors) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name, std::vector<CmagTargetProperty> properties) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {
            {
                "Debug",
                properties,
            }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget(
        "target",
        {
            {"LINK_LIBRARIES", "dep1;dep2;depExternal1"},
            {"INTERFACE_LINK_LIBRARIES", "dep4;depExternal2"},
            {"MANUALLY_ADDED_DEPENDENCIES", "depExternal3;dep3"},
        })));
    EXPECT_TRUE(project.addTarget(createTarget("dep1", {})));
    EXPECT_TRUE(project.addTarget(createTarget("dep2", {})));
    EXPECT_TRUE(project.addTarget(createTarget("dep3", {})));
    EXPECT_TRUE(project.addTarget(createTarget("dep4", {})));

    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagTarget> &targets = project.getTargets();
    ASSERT_EQ(5u, targets.size());
    const CmagTargetConfig &targetConfig = targets[0].configs[0];
    const std::vector<const CmagTarget *> expectedBuildDependencies = {&targets[1], &targets[2]};
    const std::vector<const CmagTarget *> expectedInterfaceDependencies = {&targets[4]};
    const std::vector<const CmagTarget *> expectedManualDependencies = {&targets[3]};
    const std::vector<const CmagTarget *> expectedAllDependencies = {&targets[1], &targets[2], &targets[4], &targets[3]};
    const std::vector<std::string> expectedUnmatchedDependencies = {"depExternal1", "depExternal2", "depExternal3"};
    EXPECT_EQ(expectedBuildDependencies, targetConfig.derived.buildDependencies);
    EXPECT_EQ(expectedInterfaceDependencies, targetConfig.derived.interfaceDependencies);
    EXPECT_EQ(expectedManualDependencies, targetConfig.derived.manualDependencies);
    EXPECT_EQ(expectedAllDependencies, targetConfig.derived.allDependencies);
    EXPECT_EQ(expectedUnmatchedDependencies, targetConfig.derived.unmatchedDependencies);
}

TEST_F(CmagProjectDeriveTest, givenSelfDependenciesWhenDerivingDataThenIgnoreThem) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name, std::vector<CmagTargetProperty> properties) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {
            {
                "Debug",
                properties,
            }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget(
        "A",
        {
            {"LINK_LIBRARIES", "A;B;C"},
            {"INTERFACE_LINK_LIBRARIES", "A;B;C"},
            {"MANUALLY_ADDED_DEPENDENCIES", "A;B;C"},
        })));
    EXPECT_TRUE(project.addTarget(createTarget(
        "B",
        {
            {"LINK_LIBRARIES", "C"},
            {"INTERFACE_LINK_LIBRARIES", "C"},
            {"MANUALLY_ADDED_DEPENDENCIES", "C"},
        })));
    EXPECT_TRUE(project.addTarget(createTarget(
        "C",
        {
            {"LINK_LIBRARIES", "C"},
            {"INTERFACE_LINK_LIBRARIES", "C"},
            {"MANUALLY_ADDED_DEPENDENCIES", "C"},
        })));

    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagTarget> &targets = project.getTargets();
    ASSERT_EQ(3u, targets.size());
    {
        const CmagTargetConfig &targetConfig = targets[0].configs[0];

        const std::vector<const CmagTarget *> expectedDependencies = {&targets[1], &targets[2]};
        EXPECT_EQ(expectedDependencies, targetConfig.derived.buildDependencies);
        EXPECT_EQ(expectedDependencies, targetConfig.derived.interfaceDependencies);
        EXPECT_EQ(expectedDependencies, targetConfig.derived.manualDependencies);
        EXPECT_TRUE(targetConfig.derived.unmatchedDependencies.empty());
    }
    {
        const CmagTargetConfig &targetConfig = targets[1].configs[0];

        const std::vector<const CmagTarget *> expectedDependencies = {&targets[2]};
        EXPECT_EQ(expectedDependencies, targetConfig.derived.buildDependencies);
        EXPECT_EQ(expectedDependencies, targetConfig.derived.interfaceDependencies);
        EXPECT_EQ(expectedDependencies, targetConfig.derived.manualDependencies);
        EXPECT_TRUE(targetConfig.derived.unmatchedDependencies.empty());
    }
    {
        const CmagTargetConfig &targetConfig = targets[2].configs[0];

        EXPECT_TRUE(targetConfig.derived.buildDependencies.empty());
        EXPECT_TRUE(targetConfig.derived.interfaceDependencies.empty());
        EXPECT_TRUE(targetConfig.derived.manualDependencies.empty());
        EXPECT_TRUE(targetConfig.derived.unmatchedDependencies.empty());
    }
}

TEST_F(CmagProjectDeriveTest, givenTargetWithOneConfigWhenDerivingDataThenAllPropertiesAreConsistent) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    EXPECT_TRUE(project.addTarget(CmagTarget{
        "target",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"A", "a"},
                    {"B", "b"},
                },
            },
        },
        CmagTargetGraphicalData{},
        nullptr,
        "a",
    }));
    ASSERT_EQ(1u, project.getTargets().size());
    ASSERT_TRUE(project.deriveData());

    const CmagTarget &target = project.getTargets()[0];
    const auto &propertiesDebug = target.configs[0].properties;
    EXPECT_TRUE(propertiesDebug[0].isConsistent);
    EXPECT_TRUE(propertiesDebug[1].isConsistent);
}

TEST_F(CmagProjectDeriveTest, givenTargetWithMultipleConfigsWhenDerivingDataThenDifferingPropertiesAreNotConsistent) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    EXPECT_TRUE(project.addTarget(CmagTarget{
        "target",
        CmagTargetType::Executable,
        {
            {
                "Debug",
                {
                    {"A", "a"},
                    {"B", "b"},
                    {"C", "c"},
                    {"D", "d"},
                    {"E", "e"},
                },
            },
            {
                "Release",
                {
                    {"A", "aa"},
                    {"B", "b"},
                    {"C", "cc"},
                    {"D", "d"},
                    {"E", "ee"},
                },
            },
            {
                "RelWithDeb",
                {
                    {"A", "a"},
                    {"B", "b"},
                    {"C", "c"},
                    {"D", "d"},
                    {"E", "e"},
                },
            },
        },
        CmagTargetGraphicalData{},
        nullptr,
        "a",
    }));
    ASSERT_EQ(1u, project.getTargets().size());
    ASSERT_TRUE(project.deriveData());

    const CmagTarget &target = project.getTargets()[0];

    const auto &propertiesDebug = target.configs[0].properties;
    EXPECT_FALSE(propertiesDebug[0].isConsistent);
    EXPECT_TRUE(propertiesDebug[1].isConsistent);
    EXPECT_FALSE(propertiesDebug[2].isConsistent);
    EXPECT_TRUE(propertiesDebug[3].isConsistent);
    EXPECT_FALSE(propertiesDebug[4].isConsistent);

    const auto &propertiesRelease = target.configs[1].properties;
    EXPECT_FALSE(propertiesRelease[0].isConsistent);
    EXPECT_TRUE(propertiesRelease[1].isConsistent);
    EXPECT_FALSE(propertiesRelease[2].isConsistent);
    EXPECT_TRUE(propertiesRelease[3].isConsistent);
    EXPECT_FALSE(propertiesRelease[4].isConsistent);

    const auto &propertiesRelWithDeb = target.configs[2].properties;
    EXPECT_FALSE(propertiesRelWithDeb[0].isConsistent);
    EXPECT_TRUE(propertiesRelWithDeb[1].isConsistent);
    EXPECT_FALSE(propertiesRelWithDeb[2].isConsistent);
    EXPECT_TRUE(propertiesRelWithDeb[3].isConsistent);
    EXPECT_FALSE(propertiesRelWithDeb[4].isConsistent);
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithListDirsWhenDerivingDataThenListDirIndicesAreCorrectlyDerived) {
    project.getGlobals().listDirs = {
        CmagListDir{
            "aaa",
            {1, 2},
        },
        CmagListDir{
            "aaa/bbb",
            {},
        },
        CmagListDir{
            "aaa/ccc",
            {3},
        },
        CmagListDir{
            "aaa/ccc/ddd",
            {},
        },
    };

    auto createTarget = [](const char *name, const char *listDir) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{"Debug", {}}};
        target.listDirName = listDir;
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A", "aaa")));
    EXPECT_TRUE(project.addTarget(createTarget("B", "aaa/bbb")));
    EXPECT_TRUE(project.addTarget(createTarget("C", "aaa/ccc")));
    EXPECT_TRUE(project.addTarget(createTarget("D", "aaa/ccc")));
    EXPECT_TRUE(project.addTarget(createTarget("E", "aaa")));
    EXPECT_TRUE(project.addTarget(createTarget("F", "aaa/ccc/ddd")));
    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagListDir> &listDirs = project.getGlobals().listDirs;
    EXPECT_EQ((std::vector<size_t>{0, 4}), listDirs[0].derived.targetIndices);
    EXPECT_EQ((std::vector<size_t>{1}), listDirs[1].derived.targetIndices);
    EXPECT_EQ((std::vector<size_t>{2, 3}), listDirs[2].derived.targetIndices);
    EXPECT_EQ((std::vector<size_t>{5}), listDirs[3].derived.targetIndices);
}

TEST_F(CmagProjectDeriveTest, givenVariousCMakeGeneratorsWhenDerivingDataThenDeriveCorrectGenerator) {
    ASSERT_TRUE(project.deriveData());
    EXPECT_EQ(nullptr, project.getGlobals().derived.generator);

    project.getGlobals().listDirs = {CmagListDir{"a", {}}};
    project.getGlobals().generator = "Unix Makefiles";
    ASSERT_TRUE(project.deriveData());
    EXPECT_EQ(&CMakeGenerator::unixMakefiles, project.getGlobals().derived.generator);

    project.getGlobals().listDirs = {CmagListDir{"a", {}}};
    project.getGlobals().generator = "Visual Studio 16 2019";
    ASSERT_TRUE(project.deriveData());
    EXPECT_EQ(&CMakeGenerator::vs2019, project.getGlobals().derived.generator);

    project.getGlobals().listDirs = {CmagListDir{"a", {}}};
    project.getGlobals().generator = "Bla bla";
    ASSERT_TRUE(project.deriveData());
    EXPECT_EQ(nullptr, project.getGlobals().derived.generator);
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithoutFoldersWhenDerivingDataThenAssignAllOfThemToRootFolder) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{"Debug", {}}};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A")));
    EXPECT_TRUE(project.addTarget(createTarget("B")));
    EXPECT_TRUE(project.addTarget(createTarget("C")));
    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagFolder> &folders = project.getGlobals().derived.folders;
    ASSERT_EQ(1u, folders.size());
    {
        const CmagFolder &folder = folders[0];
        EXPECT_STREQ("", folder.fullName.c_str());
        EXPECT_STREQ("", folder.relativeName.c_str());
        EXPECT_EQ(0u, folder.childIndices.size());
        EXPECT_EQ((std::vector<size_t>{0, 1, 2}), folder.targetIndices);
    }
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithEmptyFoldersWhenDerivingDataThenAssignAllOfThemToRootFolder) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{
            "Debug",
            {
                {"FOLDER", ""},
            },
        }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A")));
    EXPECT_TRUE(project.addTarget(createTarget("B")));
    EXPECT_TRUE(project.addTarget(createTarget("C")));
    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagFolder> &folders = project.getGlobals().derived.folders;
    ASSERT_EQ(1u, folders.size());
    {
        const CmagFolder &folder = folders[0];
        EXPECT_STREQ("", folder.fullName.c_str());
        EXPECT_STREQ("", folder.relativeName.c_str());
        EXPECT_EQ(0u, folder.childIndices.size());
        EXPECT_EQ((std::vector<size_t>{0, 1, 2}), folder.targetIndices);
    }
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithFoldersWhenDerivingDataThenAssignThemAccordingly) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name, const char *folder) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{
            "Debug",
            {
                {"FOLDER", folder},
            },
        }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A", "")));
    EXPECT_TRUE(project.addTarget(createTarget("B", "b")));
    EXPECT_TRUE(project.addTarget(createTarget("C", "c")));
    EXPECT_TRUE(project.addTarget(createTarget("BC1", "b/c")));
    EXPECT_TRUE(project.addTarget(createTarget("BD", "b/d")));
    EXPECT_TRUE(project.addTarget(createTarget("BDE1", "b/d/e")));
    EXPECT_TRUE(project.addTarget(createTarget("BDE2", "b/d/e")));
    EXPECT_TRUE(project.addTarget(createTarget("BC2", "b/c")));
    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagFolder> &folders = project.getGlobals().derived.folders;
    ASSERT_EQ(6u, folders.size());
    {
        const CmagFolder &folder = folders[0];
        EXPECT_STREQ("", folder.fullName.c_str());
        EXPECT_STREQ("", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{1, 2}), folder.childIndices); // { a, b }
        EXPECT_EQ((std::vector<size_t>{0}), folder.targetIndices);   // {A}
    }
    {
        const CmagFolder &folder = folders[1];
        EXPECT_STREQ("b", folder.fullName.c_str());
        EXPECT_STREQ("b", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{3, 4}), folder.childIndices); // { b/c, b/d }
        EXPECT_EQ((std::vector<size_t>{1}), folder.targetIndices);   // {B}
    }
    {
        const CmagFolder &folder = folders[2];
        EXPECT_STREQ("c", folder.fullName.c_str());
        EXPECT_STREQ("c", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{}), folder.childIndices);   // { }
        EXPECT_EQ((std::vector<size_t>{2}), folder.targetIndices); // {C}
    }
    {
        const CmagFolder &folder = folders[3];
        EXPECT_STREQ("b/c", folder.fullName.c_str());
        EXPECT_STREQ("c", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{}), folder.childIndices);      // { }
        EXPECT_EQ((std::vector<size_t>{3, 7}), folder.targetIndices); // { BC1, BC2}
    }
    {
        const CmagFolder &folder = folders[4];
        EXPECT_STREQ("b/d", folder.fullName.c_str());
        EXPECT_STREQ("d", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{5}), folder.childIndices);  // { b/d/e }
        EXPECT_EQ((std::vector<size_t>{4}), folder.targetIndices); // { BD }
    }
    {
        const CmagFolder &folder = folders[5];
        EXPECT_STREQ("b/d/e", folder.fullName.c_str());
        EXPECT_STREQ("e", folder.relativeName.c_str());
        EXPECT_EQ((std::vector<size_t>{}), folder.childIndices);      // { }
        EXPECT_EQ((std::vector<size_t>{5, 6}), folder.targetIndices); // { BDE1, BDE2}
    }
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithDependenciesWhenDerivingDataThenCorrectlyDeriveIsReferencedField) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name, const char *linkLibs, const char *interfaceLinkLibs, const char *deps) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{
            "Debug",
            {
                {"LINK_LIBRARIES", linkLibs},
                {"INTERFACE_LINK_LIBRARIES", interfaceLinkLibs},
                {"MANUALLY_ADDED_DEPENDENCIES", deps},
            },
        }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A", "", "", "")));
    EXPECT_TRUE(project.addTarget(createTarget("B", "", "", "")));
    EXPECT_TRUE(project.addTarget(createTarget("C", "", "", "")));

    // Reference A,B,C one time
    EXPECT_TRUE(project.addTarget(createTarget("X", "A", "", "")));
    EXPECT_TRUE(project.addTarget(createTarget("Y", "", "B", "")));
    EXPECT_TRUE(project.addTarget(createTarget("Z", "", "", "C")));

    // Reference X,Y,Z two times each
    EXPECT_TRUE(project.addTarget(createTarget("U", "X", "Y", "")));
    EXPECT_TRUE(project.addTarget(createTarget("V", "", "Y", "Z")));
    EXPECT_TRUE(project.addTarget(createTarget("T", "X", "", "Z")));

    // Reference U,V,T three times each
    EXPECT_TRUE(project.addTarget(createTarget("J", "U", "V", "T")));
    EXPECT_TRUE(project.addTarget(createTarget("K", "U", "V", "T")));
    EXPECT_TRUE(project.addTarget(createTarget("L", "U", "V", "T")));

    ASSERT_TRUE(project.deriveData());
    const std::vector<CmagTarget> &targets = project.getTargets();
    ASSERT_EQ(12u, targets.size());
    ASSERT_TRUE(targets[0].derived.isReferenced);
    ASSERT_TRUE(targets[1].derived.isReferenced);
    ASSERT_TRUE(targets[2].derived.isReferenced);
    ASSERT_TRUE(targets[3].derived.isReferenced);
    ASSERT_TRUE(targets[4].derived.isReferenced);
    ASSERT_TRUE(targets[5].derived.isReferenced);
    ASSERT_TRUE(targets[6].derived.isReferenced);
    ASSERT_TRUE(targets[7].derived.isReferenced);
    ASSERT_TRUE(targets[8].derived.isReferenced);
    ASSERT_FALSE(targets[9].derived.isReferenced);
    ASSERT_FALSE(targets[10].derived.isReferenced);
    ASSERT_FALSE(targets[11].derived.isReferenced);
}

TEST_F(CmagProjectDeriveTest, givenTargetsWithDependenciesWhenDerivingDataThenCorrectlyDeriveUnmatchedDependenciesField) {
    project.getGlobals().listDirs = {CmagListDir{"a", {}}};

    auto createTarget = [](const char *name, const char *linkLibs, const char *interfaceLinkLibs, const char *deps) {
        CmagTarget target = {};
        target.name = name;
        target.type = CmagTargetType::Executable;
        target.configs = {{
            "Debug",
            {
                {"LINK_LIBRARIES", linkLibs},
                {"INTERFACE_LINK_LIBRARIES", interfaceLinkLibs},
                {"MANUALLY_ADDED_DEPENDENCIES", deps},
            },
        }};
        target.listDirName = "a";
        return target;
    };

    EXPECT_TRUE(project.addTarget(createTarget("A", "B;Ext1", "Ext2;Ext3", "Ext4;C")));
    EXPECT_TRUE(project.addTarget(createTarget("B", "Ext1;Ext1", "", "Ext4;C")));
    EXPECT_TRUE(project.addTarget(createTarget("C", "", "Ext4;Ext1;Ext3;Ext2", "")));

    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagTarget> &targets = project.getTargets();
    ASSERT_EQ(3u, targets.size());
    EXPECT_EQ((std::vector<std::string>{"Ext1", "Ext2", "Ext3", "Ext4"}), targets[0].configs[0].derived.unmatchedDependencies);
    EXPECT_EQ((std::vector<std::string>{"Ext1", "Ext1", "Ext4"}), targets[1].configs[0].derived.unmatchedDependencies);
    EXPECT_EQ((std::vector<std::string>{"Ext4", "Ext1", "Ext3", "Ext2"}), targets[2].configs[0].derived.unmatchedDependencies);

    EXPECT_EQ((std::vector<std::string>{"Ext1", "Ext2", "Ext3", "Ext4"}), project.getUnmatchedDependencies());
}

struct CmagTargetConfigTest : ::testing::Test {
    static void executeTest(const char *evaledValue, const char *expectedValue) {
        executeTest(evaledValue, evaledValue, expectedValue);
    }

    static void executeTest(const char *evaledValue, const char *nonEvaledValue, const char *expectedValue) {
        CmagTargetConfig config = {
            "Debug",
            {
                {"LINK_LIBRARIES", evaledValue},
            },
        };
        config.fixupWithNonEvaled("LINK_LIBRARIES", nonEvaledValue);
        EXPECT_STREQ(expectedValue, config.properties[0].value.c_str());
    }
};

TEST_F(CmagTargetConfigTest, givenNoGenexesWhenFixingLinkLibrariesThenDoNotChangeAnything) {
    executeTest("", "", "");
    executeTest("Lib1;Lib2;Lib3", "Lib1;Lib2;Lib3", "Lib1;Lib2;Lib3");
}

TEST_F(CmagTargetConfigTest, givenSingleGenexWhenFixingLinkLibrariesThenStripIt) {
    executeTest("Lib1;Lib2;Lib3", "$<LINK_ONLY:Lib1>;Lib2;Lib3", "Lib2;Lib3");
    executeTest("Lib1;Lib2;Lib3", "Lib1;$<LINK_ONLY:Lib2>;Lib3", "Lib1;Lib3");
    executeTest("Lib1;Lib2;Lib3", "Lib1;Lib2;$<LINK_ONLY:Lib3>", "Lib1;Lib2");
    executeTest("Lib2", "$<LINK_ONLY:Lib2>", "");
}

TEST_F(CmagTargetConfigTest, givenMultipleGenexesWhenFixingLinkLibrariesThenStripThem) {
    executeTest("Lib1;Lib2", "$<LINK_ONLY:Lib1>;$<LINK_ONLY:Lib2>", "");
    executeTest("Lib1;Lib2;Lib3;Lib4", "$<LINK_ONLY:Lib1>;Lib2;$<LINK_ONLY:Lib3>;Lib4", "Lib2;Lib4");
}

TEST_F(CmagTargetConfigTest, givenNestedGenexesWhenFixingLinkLibrariesThenStripThem) {
    executeTest("Lib1;LibDebug;Lib3", "Lib1;$<LINK_ONLY:Lib$<CONFIG>>", "Lib1;Lib3");
    executeTest("Lib1;LibDebug;Lib3", "Lib1;$<LINK_ONLY:$<$<CONFIG:Debug>:LibDebug>>;Lib3", "Lib1;Lib3");
}

TEST_F(CmagTargetConfigTest, givenSingleDirectoryIdWhenFixingLinkLibrariesThenStripIt) {
    executeTest("::@(000002AAB227D1E0);Lib1;::@;Lib2;Lib3", "Lib1;Lib2;Lib3");
    executeTest("Lib1;::@(0000020E930CFDB0);Lib2;::@;Lib3", "Lib1;Lib2;Lib3");
    executeTest("Lib1;Lib2;::@(0000024CEE12F490);Lib3;::@", "Lib1;Lib2;Lib3");
    executeTest("::@(00000219A31F64F0);Lib1;::@", "Lib1");
}

TEST_F(CmagTargetConfigTest, givenMultipleeDirectoryIdsWhenFixingLinkLibrariesThenStripAllOfThem) {
    executeTest("::@(000002AAB227D1E0);Lib1;::@;Lib2;::@(000002AAB227D1E0);Lib3;::@", "Lib1;Lib2;Lib3");
    executeTest("::@(000002AAB227D1E0);Lib1;::@;::@(000002AAB227D1E0);Lib2;::@;Lib3", "Lib1;Lib2;Lib3");
}

TEST_F(CmagTargetConfigTest, givenDirectoryIdAndLinkOnlyWhenFixingLinkLibrariesThenStripBoth) {
    executeTest("::@(000002F0C2555640);Lib1;::@", "::@(000002F0C2555640);$<LINK_ONLY:Lib1>;::@", "");
    executeTest("::@(000002F0C2555640);Lib1;::@;Lib2", "::@(000002F0C2555640);$<LINK_ONLY:Lib1>;::@;Lib2", "Lib2");
}
