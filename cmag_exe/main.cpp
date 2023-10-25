#include "cmag_lib/cmake_generation/argument_parser.h"
#include "cmag_lib/cmake_generation/generate.h"
#include "cmag_lib/utils/error.h"

int main(int argc, const char **argv) {
    // Parse arguments
    ArgumentParser argParser{argc, argv};
    if (!argParser.isValid()) {
        printf("<help message>\n");
        return 1;
    }
    const fs::path &sourcePath = argParser.getSourcePath();
    if (sourcePath.empty()) {
        LOG_ERROR("empty source dir.");
        return 1;
    }
    if (!fs::is_directory(sourcePath)) {
        LOG_ERROR("not a valid source dir.");
        return 1;
    }

    // Call CMake and gather information about the build system
    return generateCmake(argParser);
}