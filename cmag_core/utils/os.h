#pragma once

#include "cmag_core/utils/enum_utils.h"

enum class OperatingSystem {
    Windows = 1,
    Linux = 2,
    Osx = 4,
};

BITFIELD_ENUM(OperatingSystem)

#ifdef __linux__

#define CMAG_OS (OperatingSystem::Linux)

#elif _WIN32

#define CMAG_OS windows

#define CMAG_OS (OperatingSystem::Windows)

#else

#error "Unsupported OS"

#endif
