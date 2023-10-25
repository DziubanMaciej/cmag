#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/subprocess.h"

#include <Windows.h>
#include <sstream>

SubprocessResult runSubprocess(int argc, const char **argv) {
    // Get application name
    FATAL_ERROR_IF(argc == 0);
    char applicationName[4096];
    if (::SearchPathA(NULL, argv[0], ".exe", sizeof(applicationName), applicationName, NULL) == 0) {
        return SubprocessResult::PathResolvingFailed;
    }

    // Prepare arguments
    FATAL_ERROR_IF(argv[argc - 1] != nullptr, "Last arg should be nullptr"); // we will skip this arg
    std::ostringstream fullCommandLineStream;
    for (int i = 1; i < argc - 1; i++) {
        fullCommandLineStream << argv[i] << ' ';
    }
    std::string fullCommandLine = fullCommandLineStream.str();

    PROCESS_INFORMATION processInfo = {};

    STARTUPINFO startup_info = {};
    startup_info.cb = sizeof(STARTUPINFO);

    BOOL success = ::CreateProcessA(
        applicationName,
        fullCommandLine.data(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startup_info,
        &processInfo);
    if (success == FALSE) {
        return SubprocessResult::CreationFailed;
    }

    if (::WaitForSingleObject(processInfo.hProcess, INFINITE) != WAIT_OBJECT_0) {
        return SubprocessResult::ProcessKilled;
    }

    DWORD exitCode{};
    if (::GetExitCodeProcess(processInfo.hProcess, &exitCode) == 0) {
        return SubprocessResult::ProcessKilled;
    }

    ::CloseHandle(processInfo.hProcess);
    ::CloseHandle(processInfo.hThread);

    return exitCode == 0 ? SubprocessResult::Success : SubprocessResult::ProcessFailed;
}
