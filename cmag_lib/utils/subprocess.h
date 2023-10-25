#pragma once

enum class SubprocessResult {
    Success,
    CreationFailed,
    ProcessKilled,
    ProcessFailed,
    PathResolvingFailed,
};

SubprocessResult runSubprocess(int argc, const char **argv);
