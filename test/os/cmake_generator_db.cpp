#include "cmake_generator_db.h"

CmakeGeneratorDb::CmakeGeneratorDb(bool allGenerators) {
    CMakeGenerator possibleGenerators[] = {
#if _WIN32
        {
            "Visual Studio 17 2022",
            "Vs2022",
            true,
            true,
        },
        {
            "Visual Studio 16 2019",
            "Vs2019",
            true,
            true,
        },
        {
            "Visual Studio 15 2017",
            "Vs2017",
            true,
            true,
        },
#endif
        {
            "Ninja Multi-Config",
            "NinjaMc",
            true,
            false,
        },
#ifdef __linux__
        {
            "Unix Makefiles",
            "UnixMake",
            false,
            false,
        },
#endif
        {
            "Ninja",
            "Ninja",
            false,
            false,
        },
#if _WIN32
        {
            "MinGW Makefiles",
            "MingwMake",
            false,
            false,
        },
#endif
    };

    bool hasVisualStudio = false;
    bool hasMultiConfig = false;
    bool hasSingleConfig = false;
    for (const CMakeGenerator &generator : possibleGenerators) {
        // Skip if we already have this kind of generator
        if (generator.isVisualStudio && hasVisualStudio) {
            continue;
        }
        if (generator.isMultiConfig && hasMultiConfig && !allGenerators) {
            continue;
        }
        if (!generator.isMultiConfig && hasSingleConfig && !allGenerators) {
            continue;
        }

        // Mark already seen generators
        hasVisualStudio |= generator.isVisualStudio;
        hasMultiConfig |= generator.isMultiConfig;
        hasSingleConfig |= (!generator.isMultiConfig);

        // Perform a dummy CMake run to see, whether this generator works. If it does,
        // add it to our list.
        if (isGeneratorAvailable(generator.name.c_str())) {
            generators.push_back(generator);
        }
    }
}

bool CmakeGeneratorDb::isGeneratorAvailable(const char *name) {
    std::vector<std::string> args{
        "cmake",
        "-G",
        name,
    };
    std::string stdOut{};
    std::string stdErr{};
    runSubprocess(args, stdOut, stdErr);

    return stdErr.find("CMake Error: Could not create named generator") == std::string::npos;
}
