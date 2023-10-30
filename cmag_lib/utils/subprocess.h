#pragma once

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
