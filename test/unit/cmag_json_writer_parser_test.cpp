#include "cmag_lib/json/cmag_json_parser.h"
#include "cmag_lib/json/cmag_json_writer.h"

#include <gtest/gtest.h>

struct CmagWriterParserTest : ::testing::Test {
    void verify(const CmagProject &initialProject) {
        std::ostringstream jsonStream;
        CmagJsonWriter::writeProject(initialProject, jsonStream);
        std::string json = jsonStream.str();

        CmagProject derivedProject{};
        ASSERT_EQ(ParseResult::Success, CmagJsonParser::parseProject(json.c_str(), derivedProject));
        compareProjects(initialProject, derivedProject);
    }

    void compareProjects(const CmagProject &exp, const CmagProject &act) {
        // This function naively assumes that the order of configs and/or properties will not change.
        // While this is completely uninmportant for json parsing, maintaining stable order greatly
        // eases testing. To achieve this, we have to put everything alphabetically, e.g. "Debug" config
        // has to be before "Release" config and "propertyL" has to be before "propertyM".

        EXPECT_EQ(exp.getConfigs(), act.getConfigs());
        EXPECT_EQ(exp.getGlobals().darkMode, act.getGlobals().darkMode);

        const auto &expTargets = exp.getTargets();
        const auto &actTargets = act.getTargets();
        ASSERT_EQ(expTargets.size(), actTargets.size());
        for (size_t i = 0; i < expTargets.size(); i++) {
            const auto &expTarget = expTargets[i];
            const auto &actTarget = actTargets[i];

            EXPECT_EQ(expTarget.name, actTarget.name);
            EXPECT_EQ(expTarget.type, actTarget.type);

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
        }
    }
};

TEST_F(CmagWriterParserTest, givenProjectWithNoTargetsThenWriteAndReadCorrectly) {
    CmagProject project;
    project.getGlobals().darkMode = false;
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithoutPropertiesThenWriteAndReadCorrectly) {
    CmagProject project;
    project.getGlobals().darkMode = false;
    project.addTarget(CmagTarget{
        "myTarget",
        CmagTargetType::Executable,
        {
            {"Debug", {}},
        },
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithPropertiesThenWriteAndReadCorrectly) {
    CmagProject project;
    project.getGlobals().darkMode = false;
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
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithMultipleConfigsThenWriteAndReadCorrectly) {
    CmagProject project;
    project.getGlobals().darkMode = false;
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
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithTargetWithMultipleConfigsAndDifferingPropertiesThenWriteAndReadCorrectly) {
    CmagProject project;
    project.getGlobals().darkMode = false;
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
    });
    verify(project);
}

TEST_F(CmagWriterParserTest, givenProjectWithVariousTargetTypesThenWriteAndReadCorrectly) {
    CmagTargetType types[] = {
        CmagTargetType::StaticLibrary,
        CmagTargetType::ModuleLibrary,
        CmagTargetType::SharedLibrary,
        CmagTargetType::ObjectLibrary,
        CmagTargetType::InterfaceLibrary,
        CmagTargetType::Executable,
    };
    for (CmagTargetType type : types) {
        CmagProject project;
        project.getGlobals().darkMode = false;
        project.addTarget(CmagTarget{
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
        });
        verify(project);
    }
}
