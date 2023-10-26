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
    };

    CmagTarget target2 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
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
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
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
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
            {"Debug", {}},
        },
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
    };

    CmagTarget target2 = {
        "target",
        CmagTargetType::StaticLibrary,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
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
    };

    CmagTarget target2 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Release", {}},
            {"ReleaseWithDebInfo", {}},
        },
    };

    CmagTarget target3 = {
        "target2",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
            {"MinSizeRel", {}},
        },
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
    };

    CmagTargetConfig &queriedConfig = target.getOrCreateConfig("RelWithDebInfo");
    EXPECT_EQ(&target.configs[2], &queriedConfig);
    EXPECT_EQ(3u, target.configs.size());
    EXPECT_STREQ("RelWithDebInfo", queriedConfig.name.c_str());
    EXPECT_EQ(0u, queriedConfig.properties.size());
}
