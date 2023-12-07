#include "cmag_core/utils/filesystem.h"

#include <fstream>
#include <optional>
#include <sstream>

inline std::optional<std::string> readFile(const fs::path &path) {
    std::ifstream stream(path);
    if (!stream) {
        return {};
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}