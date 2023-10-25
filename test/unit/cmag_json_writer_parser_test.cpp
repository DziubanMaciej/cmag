#include "cmag_lib/core/cmag_json_parser.h"
#include "cmag_lib/core/cmag_json_writer.h"

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

            ASSERT_EQ(expTarget.properties.size(), actTarget.properties.size());
            for (size_t j = 0; j < expTarget.properties.size(); j++) {
                const auto &expPropertiesForConfig = expTarget.properties[j];
                const auto &actPropertiesForConfig = actTarget.properties[j];

                EXPECT_EQ(expPropertiesForConfig.first, actPropertiesForConfig.first);
                EXPECT_EQ(expPropertiesForConfig.second.size(), actPropertiesForConfig.second.size());
                for (size_t k = 0; k < expPropertiesForConfig.second.size(); k++) {
                    const auto &expProperty = expPropertiesForConfig.second[k];
                    const auto &actProperty = actPropertiesForConfig.second[k];
                    EXPECT_EQ(expProperty.first, actProperty.first);
                    EXPECT_EQ(expProperty.second, actProperty.second);
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
