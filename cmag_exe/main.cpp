#include "cmag_lib/cmag.h"
#include "cmag_lib/cmake_generation/argument_parser.h"
#include "cmag_lib/utils/error.h"

#define RETURN_ERROR(expr)                \
    do {                                  \
        const CmagResult r = (expr);      \
        if ((r) != CmagResult::Success) { \
            return static_cast<int>(r);   \
        }                                 \
    } while (false)

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

    Cmag cmag{"project"};
    RETURN_ERROR(cmag.generateCmake(argParser.getSourcePath(), argParser.constructArgsForCmake()));
    RETURN_ERROR(cmag.readCmagProjectFromGeneration(argParser.getBuildPath()));
    RETURN_ERROR(cmag.generateGraphPositionsForProject(argParser.getBuildPath(), argParser.getGraphvizPath()));
    RETURN_ERROR(cmag.writeProjectToFile(argParser.getBuildPath()));
    return 0;
}
