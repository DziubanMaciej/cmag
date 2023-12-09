#include "cmag_core/core/version.h"
#include "cmag_core/dumper/cmag_dumper.h"
#include "cmag_core/dumper/dumper_argument_parser.h"
#include "cmag_core/utils/error.h"

#define RETURN_ERROR(expr)                \
    do {                                  \
        const CmagResult r = (expr);      \
        if ((r) != CmagResult::Success) { \
            return static_cast<int>(r);   \
        }                                 \
    } while (false)

int main(int argc, const char **argv) {
    // Parse arguments
    DumperArgumentParser argParser{argc, argv};
    if (argParser.getShowVersion()) {
        LOG_INFO(cmagVersion.toString());
        return 0;
    }
    if (!argParser.isValid()) {
        argParser.printHelp();
        return 1;
    }
    const fs::path &sourcePath = argParser.getSourcePath();
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

    CmagDumper dumper{argParser.getProjectName(), argParser.getJsonDebug()};
    RETURN_ERROR(dumper.generateCmake(argParser.getSourcePath(), argParser.getBuildPath(), argParser.constructArgsForCmake(), argParser.getExtraTargetProperties()));
    RETURN_ERROR(dumper.readCmagProjectFromGeneration(argParser.getBuildPath()));
    RETURN_ERROR(dumper.generateGraphPositionsForProject(argParser.getBuildPath(), argParser.getGraphvizPath()));
    RETURN_ERROR(dumper.writeProjectToFile(argParser.getBuildPath()));
    if (!argParser.getJsonDebug()) {
        RETURN_ERROR(dumper.cleanupTemporaryFiles());
    }
    if (argParser.getLaunchGui()) {
        RETURN_ERROR(dumper.launchProjectInGui(argParser.getBuildPath()));
    }
    return 0;
}
