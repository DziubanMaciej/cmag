#include "cmag_core/utils/error.h"
#include "cmag_core/utils/filesystem.h"

#include <Windows.h>
#include <shellapi.h>

fs::path getExeLocation() {
    char buffer[4096];
    DWORD result = GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    if (result == sizeof(buffer) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        FATAL_ERROR("Insufficient buffer for getExeLocation");
    }

    return buffer;
}

bool openHyperlink(const char *path) {
    HINSTANCE result = ::ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
    return reinterpret_cast<INT_PTR>(result) > 32; // yes, this is what the documentation says...
}
