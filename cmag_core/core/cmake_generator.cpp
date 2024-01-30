#include "cmake_generator.h"

const CMakeGenerator CMakeGenerator::vs2017{
    "Visual Studio 15 2017\"",
    "Vs2017",
    OperatingSystem::Windows,
    true,
    true,
    true,
};

const CMakeGenerator CMakeGenerator::vs2019{
    "Visual Studio 16 2019",
    "Vs2019",
    OperatingSystem::Windows,
    true,
    true,
    true,
};

const CMakeGenerator CMakeGenerator::vs2022{
    "Visual Studio 17 2022",
    "Vs2022",
    OperatingSystem::Windows,
    true,
    true,
    true,
};

const CMakeGenerator CMakeGenerator::ninja{
    "Ninja",
    "Ninja",
    OperatingSystem::Windows | OperatingSystem::Linux,
    false,
    false,
    false,
};

const CMakeGenerator CMakeGenerator::ninjaMultiConfig{
    "Ninja Multi-Config",
    "NinjaMc",
    OperatingSystem::Windows | OperatingSystem::Linux,
    true,
    false,
    false,
};

const CMakeGenerator CMakeGenerator::unixMakefiles{
    "Unix Makefiles",
    "UnixMake",
    OperatingSystem::Linux | OperatingSystem::Osx,
    false,
    false,
    false,
};

const CMakeGenerator CMakeGenerator::minGwMakefiles{
    "MinGW Makefiles",
    "MingwMake",
    OperatingSystem::Windows,
    false,
    false,
    false,
};

const CMakeGenerator CMakeGenerator::xcode{
    "Xcode",
    "Xcode",
    OperatingSystem::Osx,
    false,
    false,
    false,
};
