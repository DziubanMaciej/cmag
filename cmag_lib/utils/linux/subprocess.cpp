#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/subprocess.h"

#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#define FATAL_ERROR_ON_FAILED_SYSCALL(expression)                                 \
    if ((expression) < 0) {                                                       \
        FATAL_ERROR("Syscall error on \"", #expression, "\", ", strerror(errno)); \
    }

SubprocessResult runSubprocess(const std::vector<std::string> &args) {
    int forkResult = fork();
    if (forkResult == -1) {
        return SubprocessResult::CreationFailed;
    }

    if (forkResult == 0) {
        // Child

        // Prepare args
        std::vector<const char*> argv= {};
        argv.reserve(args.size() + 1);
        for (const std::string &arg : args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);

        // Call exec
        char *const *argvCasted = const_cast<char *const *>(argv.data()); // we can mess with const correctness, since we're calling exec anyway.
        FATAL_ERROR_ON_FAILED_SYSCALL(execvp(argv[0], argvCasted));
        FATAL_ERROR("Unreachable code");
    } else {
        // Parent
        int pid = forkResult;

        // Wait for finish
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
}
