#include "cmag_lib/utils/error.h"
#include "cmag_lib/utils/filesystem.h"

#include <Windows.h>

fs::path getExeLocation() {
    char buffer[4096];
    DWORD result = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    if (result == sizeof(buffer) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        FATAL_ERROR("Insufficient buffer for getExeLocation");
    }

    return buffer;
}
