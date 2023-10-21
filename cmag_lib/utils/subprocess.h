#pragma once

enum class SubprocessResult {
    Success,
    CreationFailed,
    ProcessKilled,
    ProcessFailed,
};

SubprocessResult runSubprocess(int argc, const char **argv);
