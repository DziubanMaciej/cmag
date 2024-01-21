#include "cmag_core/utils/subprocess.h"

#include <Windows.h>
#include <sstream>

#define RETURN_ERROR(expr)                      \
    do {                                        \
        const SubprocessResult r = (expr);      \
        if ((r) != SubprocessResult::Success) { \
            return r;                           \
        }                                       \
    } while (false)

static SubprocessResult prepareArgsForCreateProcess(const std::vector<std::string> &args, std::string &outAppName, std::string &outCmdline) {
    // Get application name
    FATAL_ERROR_IF(args.empty());
    char applicationName[4096];
    if (::SearchPathA(NULL, args[0].c_str(), ".exe", sizeof(applicationName), applicationName, NULL) == 0) {
        return SubprocessResult::PathResolvingFailed;
    }

    // Prepare arguments
    std::ostringstream fullCommandLineStream = {};
    for (const std::string &arg : args) {
        fullCommandLineStream << "\"" << arg << "\" ";
    }

    outAppName = applicationName;
    outCmdline = fullCommandLineStream.str();
    return SubprocessResult::Success;
}

static SubprocessResult waitForResult(PROCESS_INFORMATION &processInfo) {
    if (::WaitForSingleObject(processInfo.hProcess, INFINITE) != WAIT_OBJECT_0) {
        ::CloseHandle(processInfo.hProcess);
        ::CloseHandle(processInfo.hThread);
        return SubprocessResult::ProcessKilled;
    }

    DWORD exitCode{};
    FATAL_ERROR_IF(::GetExitCodeProcess(processInfo.hProcess, &exitCode) == 0);

    return exitCode == 0 ? SubprocessResult::Success : SubprocessResult::ProcessFailed;
}

static SubprocessResult createPipeForStd(STARTUPINFO &startupInfo, bool isStderr, HANDLE &readHandle, HANDLE &writeHandle) {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (CreatePipe(&readHandle, &writeHandle, &saAttr, 0) == 0) {
        return SubprocessResult::CreationFailed;
    }

    if (isStderr) {
        startupInfo.hStdError = writeHandle;
    } else {
        startupInfo.hStdOutput = writeHandle;
    }
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    return SubprocessResult::Success;
}

void readPipeForStd(HANDLE readHandle, std::string &outContent) {
    CHAR buffer[4096];
    DWORD bytesRead;
    while (true) {
        if (!ReadFile(readHandle, buffer, sizeof(buffer), &bytesRead, NULL) || bytesRead == 0) {
            break;
        }
        outContent.append(buffer, bytesRead);
    }
}

SubprocessResult runSubprocess(const std::vector<std::string> &args) {
    std::string appName;
    std::string cmdLine;
    RETURN_ERROR(prepareArgsForCreateProcess(args, appName, cmdLine));

    PROCESS_INFORMATION processInfo = {};

    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(STARTUPINFO);

    BOOL success = ::CreateProcessA(
        appName.c_str(),
        cmdLine.data(),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);
    if (success == FALSE) {
        return SubprocessResult::CreationFailed;
    }

    return waitForResult(processInfo);
}

SubprocessResult runSubprocess(const std::vector<std::string> &args, std::string &stdOut, std::string &stdErr) {
    std::string appName;
    std::string cmdLine;
    RETURN_ERROR(prepareArgsForCreateProcess(args, appName, cmdLine));

    PROCESS_INFORMATION processInfo = {};

    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(STARTUPINFO);
    HANDLE pipeStdoutRead = INVALID_HANDLE_VALUE;
    HANDLE pipeStdoutWrite = INVALID_HANDLE_VALUE;
    HANDLE pipeStderrRead = INVALID_HANDLE_VALUE;
    HANDLE pipeStderrWrite = INVALID_HANDLE_VALUE;
    RETURN_ERROR(createPipeForStd(startupInfo, false, pipeStdoutRead, pipeStdoutWrite));
    RETURN_ERROR(createPipeForStd(startupInfo, true, pipeStderrRead, pipeStderrWrite));

    BOOL success = ::CreateProcessA(
        appName.c_str(),
        cmdLine.data(),
        nullptr,
        nullptr,
        true,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &startupInfo,
        &processInfo);
    ::CloseHandle(pipeStdoutWrite);
    ::CloseHandle(pipeStderrWrite);
    if (success == FALSE) {
        ::CloseHandle(pipeStdoutRead);
        ::CloseHandle(pipeStderrRead);
        return SubprocessResult::CreationFailed;
    }

    const SubprocessResult result = waitForResult(processInfo);

    readPipeForStd(pipeStdoutRead, stdOut);
    readPipeForStd(pipeStderrRead, stdErr);

    ::CloseHandle(processInfo.hProcess);
    ::CloseHandle(processInfo.hThread);

    ::CloseHandle(pipeStdoutRead);
    ::CloseHandle(pipeStderrRead);

    return result;
}
