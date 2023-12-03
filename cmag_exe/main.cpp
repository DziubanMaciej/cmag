#include "cmag_lib/core/argument_parser.h"
#include "cmag_lib/core/cmag.h"
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
    const fs::path &sourcePath = argParser.getSourcePath();
    if (!argParser.isValid()) {
        argParser.printHelp();
        return 1;
    }
    if (sourcePath.empty()) {
        LOG_ERROR("empty source dir.\n");
        argParser.printHelp();
        return 1;
    }
    if (!fs::is_directory(sourcePath)) {
        LOG_ERROR("not a valid source dir.\n");
        argParser.printHelp();
        return 1;
    }

    Cmag cmag{argParser.getProjectName()};
    RETURN_ERROR(cmag.generateCmake(argParser.getSourcePath(), argParser.getBuildPath(), argParser.constructArgsForCmake(), argParser.getExtraTargetProperties(), argParser.getJsonDebug()));
    RETURN_ERROR(cmag.readCmagProjectFromGeneration(argParser.getBuildPath()));
    RETURN_ERROR(cmag.generateGraphPositionsForProject(argParser.getBuildPath(), argParser.getGraphvizPath()));
    RETURN_ERROR(cmag.writeProjectToFile(argParser.getBuildPath()));
    if (argParser.getLaunchGui()) {
        RETURN_ERROR(cmag.launchProjectInGui(argParser.getBuildPath()));
    }
    return 0;
}
