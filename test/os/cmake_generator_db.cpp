#include "cmake_generator_db.h"

#include "test/os/utils/test_workspace.h"

CmakeGeneratorDb::CmakeGeneratorDb(bool allGenerators) {

    bool hasVisualStudio = false;
    bool hasMultiConfig = false;
    bool hasSingleConfig = false;
    for (const CMakeGenerator *generator : CMakeGenerator::allGenerators) {
        // Skip if this generator is unsupported on current system
        if (!hasOperatingSystemBit(generator->supportedSystems, CMAG_OS)) {
            continue;
        }

        // Skip if we already have this kind of generator
        if (generator->isVisualStudio && hasVisualStudio) {
            continue;
        }
        if (generator->isMultiConfig && hasMultiConfig && !allGenerators) {
            continue;
        }
        if (!generator->isMultiConfig && hasSingleConfig && !allGenerators) {
            continue;
        }

        // Perform a dummy CMake run to see, whether this generator works. If it does,
        // add it to our list.
        printf("%s available: ", generator->gtestName.c_str());
        if (isGeneratorAvailable(generator->name.c_str())) {
            printf("YES\n");
            generators.push_back(*generator);

            // Mark already seen generators
            hasVisualStudio |= generator->isVisualStudio;
            hasMultiConfig |= generator->isMultiConfig;
            hasSingleConfig |= (!generator->isMultiConfig);
        } else {
            printf("NO\n");
        }
    }
}

bool CmakeGeneratorDb::isGeneratorAvailable(const char *name) {
    TestWorkspace workspace = TestWorkspace::prepare("hello_world");

    std::vector<std::string> args{
        "cmake",
        "-B",
        workspace.buildPath.string(),
        "-S",
        workspace.sourcePath.string(),
        "-G",
        name,
    };

    std::string stdOut{};
    std::string stdErr{};
    const SubprocessResult result = runSubprocess(args, stdOut, stdErr);
    switch (result) {
    case SubprocessResult::Success:
        return true;
    case SubprocessResult::ProcessFailed:
        return false;
    default:
        FATAL_ERROR("Unexepected result when checking generator availability: ", int(result))
    }
}
