#include "cmag_lib/core/cmag_project.h"

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

TEST(CmagProjectTest, givenTargetsWithDependenciesWhenDerivingDataThenDependenciesAreSavedToTheVectors) {
    CmagProject project = {};

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
            {"LINK_LIBRARIES", "dep1;dep2"},
            {"MANUALLY_ADDED_DEPENDENCIES", "depExternal;dep3"},
        })));
    EXPECT_TRUE(project.addTarget(createTarget("dep1", {})));
    EXPECT_TRUE(project.addTarget(createTarget("dep2", {})));
    EXPECT_TRUE(project.addTarget(createTarget("dep3", {})));

    ASSERT_TRUE(project.deriveData());

    const std::vector<CmagTarget> &targets = project.getTargets();
    ASSERT_EQ(4u, targets.size());
    const CmagTargetConfig &targetConfig = targets[0].configs[0];
    const std::vector<const CmagTarget *> expectedLinkDependencies = {&targets[1], &targets[2]};
    const std::vector<const CmagTarget *> expectedBuildDependencies = {&targets[1], &targets[2], &targets[3]};
    EXPECT_EQ(expectedLinkDependencies, targetConfig.derived.linkDependencies);
    EXPECT_EQ(expectedBuildDependencies, targetConfig.derived.buildDependencies);
}

TEST(CmagProjectTest, givenTargetWithOneConfigWhenDerivingDataThenAllPropertiesAreConsistent) {
    CmagProject project = {};

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

TEST(CmagProjectTest, givenTargetWithMultipleConfigsWhenDerivingDataThenDifferingPropertiesAreNotConsistent) {
    CmagProject project = {};

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

TEST(CmagProjectTest, givenTargetsWithListDirsWhenDerivingDataThenListDirIndicesAreCorrectlyDerived) {
    CmagProject project = {};

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

struct CmagTargetConfigTest : ::testing::Test {
    void executeTest(const char *evaledValue, const char *expectedValue) {
        executeTest(evaledValue, evaledValue, expectedValue);
    }

    void executeTest(const char *evaledValue, const char *nonEvaledValue, const char *expectedValue) {
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
