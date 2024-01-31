#pragma once

#include "cmag_core/utils/os.h"

#include <string>

struct CMakeGenerator {
    std::string name;
    std::string gtestName;
    OperatingSystem supportedSystems;
    bool isMultiConfig;
    bool isVisualStudio;
    bool supportsTargetFolders;

    const static CMakeGenerator vs2017;
    const static CMakeGenerator vs2019;
    const static CMakeGenerator vs2022;
    const static CMakeGenerator ninja;
    const static CMakeGenerator ninjaMultiConfig;
    const static CMakeGenerator unixMakefiles;
    const static CMakeGenerator minGwMakefiles;
    const static CMakeGenerator xcode;

    static inline const CMakeGenerator *allGenerators[] = {
        &vs2022,
        &vs2019,
        &vs2017,
        &ninjaMultiConfig,
        &unixMakefiles,
        &ninja,
        &minGwMakefiles,
        &xcode,
    };
};