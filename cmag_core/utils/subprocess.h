#pragma once

#include "cmag_core/utils/linux/error.h"

#include <sstream>
#include <string>
#include <vector>

enum class SubprocessResult {
    Success,
    CreationFailed,
    ProcessKilled,
    ProcessFailed,
    PathResolvingFailed,
};

SubprocessResult runSubprocess(const std::vector<std::string> &args);
SubprocessResult runSubprocess(const std::vector<std::string> &args, std::string &stdOut, std::string &stdErr);

inline std::string subprocessResultToString(SubprocessResult result, const char *binaryName) {
    std::ostringstream stream = {};

    switch (result) {
    case SubprocessResult::Success:
        stream << "Running " << binaryName << " succeeded.";
        break;
    case SubprocessResult::CreationFailed:
        stream << "Failed to launch " << binaryName << '.';
        break;
    case SubprocessResult::ProcessKilled:
        stream << binaryName << " has been killed.";
        break;
    case SubprocessResult::ProcessFailed:
        stream << binaryName << " failed.";
        break;
    case SubprocessResult::PathResolvingFailed:
        stream << "Failed to resolve path for " << binaryName << '.';
        break;
    default:
        FATAL_ERROR("Invalid SubprocessResult.")
    }

    return stream.str();
}
