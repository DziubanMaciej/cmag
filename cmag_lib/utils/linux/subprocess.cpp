#include "cmag_lib/utils/linux/error.h"
#include "cmag_lib/utils/subprocess.h"

#include <sys/wait.h>
#include <unistd.h>

static SubprocessResult waitForResult(int pid) {
    int status{};
    int exitCode = -1;
    while (true) {
        int waitResult = waitpid(pid, &status, 0);

        FATAL_ERROR_ON_FAILED_SYSCALL(waitResult);
        if (WIFSIGNALED(status)) {
            return SubprocessResult::ProcessKilled;
        }
        if (WIFEXITED(status)) {
            exitCode = WEXITSTATUS(status);
            break;
        }
    }

    if (exitCode != 0) {
        return SubprocessResult::ProcessFailed;
    }
    return SubprocessResult::Success;
}

std::vector<char *> prepareArgsForExecvp(const std::vector<std::string> &args) {
    std::vector<char *> argv = {};
    argv.reserve(args.size() + 1);
    for (const std::string &arg : args) {
        // we can mess with const correctness, since we're calling exec anyway.
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    return argv;
}

SubprocessResult runSubprocess(const std::vector<std::string> &args) {
    int forkResult = fork();
    if (forkResult == -1) {
        return SubprocessResult::CreationFailed;
    }

    if (forkResult == 0) {
        // Child
        std::vector<char *> argsPrepared = prepareArgsForExecvp(args);
        FATAL_ERROR_ON_FAILED_SYSCALL(execvp(argsPrepared[0], argsPrepared.data()));
        FATAL_ERROR("Unreachable code");
    } else {
        // Parent
        return waitForResult(forkResult);
    }
}

SubprocessResult runSubprocess(const std::vector<std::string> &args, std::string &stdOut, std::string &stdErr) {
    int pipe_stdout[2] = {};
    int pipe_stderr[2] = {};
    FATAL_ERROR_ON_FAILED_SYSCALL(pipe(pipe_stdout));
    FATAL_ERROR_ON_FAILED_SYSCALL(pipe(pipe_stderr));

    int forkResult = fork();
    if (forkResult == -1) {
        return SubprocessResult::CreationFailed;
    }

    if (forkResult == 0) {
        // Child

        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stdout[0]));
        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stderr[0]));
        FATAL_ERROR_ON_FAILED_SYSCALL(dup2(pipe_stdout[1], STDOUT_FILENO));
        FATAL_ERROR_ON_FAILED_SYSCALL(dup2(pipe_stderr[1], STDERR_FILENO));
        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stdout[1]));
        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stderr[1]));

        std::vector<char *> argsPrepared = prepareArgsForExecvp(args);
        FATAL_ERROR_ON_FAILED_SYSCALL(execvp(argsPrepared[0], argsPrepared.data()));
        FATAL_ERROR("Unreachable code");
    } else {
        // Parent

        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stdout[1]));
        FATAL_ERROR_ON_FAILED_SYSCALL(close(pipe_stderr[1]));

        SubprocessResult result = waitForResult(forkResult);

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipe_stdout[0], buffer, sizeof(buffer))) > 0) {
            stdOut += std::string(buffer, bytesRead);
        }

        while ((bytesRead = read(pipe_stderr[0], buffer, sizeof(buffer))) > 0) {
            stdErr += std::string(buffer, bytesRead);
        }

        return result;
    }
}