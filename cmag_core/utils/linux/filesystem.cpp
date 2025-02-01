#include "cmag_core/utils/filesystem.h"
#include "cmag_core/utils/linux/error.h"
#include "cmag_core/utils/string_utils.h"

#include <unistd.h>

fs::path getExeLocation() {
    char buffer[4096];
    const ssize_t result = readlink("/proc/self/exe", buffer, sizeof(buffer));
    FATAL_ERROR_ON_FAILED_SYSCALL(result);
    if (result < static_cast<int64_t>(sizeof(buffer))) {
        buffer[result] = 0;
    } else {
        FATAL_ERROR_ON_FAILED_SYSCALL(-1); // to small buffer
    }
    return buffer;
}

bool openHyperlink(const char *path) {
    FORMAT_STRING(command, "xdg-open \"%s\"", path);
    return system(command) == 0;
}
