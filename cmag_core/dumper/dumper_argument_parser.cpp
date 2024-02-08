#include "dumper_argument_parser.h"

#include "cmag_core/utils/error.h"

#include <cstring>

DumperArgumentParser::DumperArgumentParser(int argc, const char **argv) : argc(argc), argv(argv) {
    if (argc == 1) {
        valid = false;
        return;
    }
    int argIndex = 0;

    // Skip Cmag process name
    argIndex++;

    // First try to parse Cmag-specific arguments. The first argument that does not start with a dash will be treated
    // as Cmake command and the rest will be args to Cmake.
    for (; argIndex < argc; argIndex++) {
        const std::string arg = argv[argIndex];
        const char *nextArg = (argIndex + 1) < argc ? argv[argIndex + 1] : nullptr;

        // Ignore empty args
        if (arg.empty()) {
            continue;
        }

        // Check for end of Cmag args
        if (arg[0] != '-') {
            break;
        }

        // Parse arguments known to us
        bool validArg = false;
        if (const char *value = parseKeyValueArgument("-p", argIndex, arg, nextArg); value) {
            projectName = value;
            validArg = true;
        }
        if (const char *value = parseKeyValueArgument("-e", argIndex, arg, nextArg); value) {
            if (!extraTargetProperties.empty()) {
                extraTargetProperties.push_back(';');
            }
            extraTargetProperties += value;
            validArg = true;
        }
        if (arg == "-v") {
            showVersion = true;
            validArg = true;
        }
        if (arg == "-d") {
            jsonDebug = true;
            validArg = true;
        }
        if (arg == "-g") {
            launchGui = true;
            validArg = true;
        }
        if (arg == "-f") {
            makeFindPackageGlobal = true;
            validArg = true;
        }

        // Check validity of current arg
        if (!validArg) {
            valid = false;
        }
    }

    // Skip CMake process name
    cmakeArgsStartIndex = argIndex;
    argIndex++;

    // Process CMake arguments
    for (; argIndex < argc; argIndex++) {
        std::string arg = argv[argIndex];
        const char *nextArg = (argIndex + 1) < argc ? argv[argIndex + 1] : nullptr;

        // Ignore empty args
        if (arg.empty()) {
            continue;
        }

        // Handle arguments that we care about
        if (const char *value = parseKeyValueArgument("-S", argIndex, arg, nextArg); value) {
            sourcePath = value;
            continue;
        }
        if (const char *value = parseKeyValueArgument("-B", argIndex, arg, nextArg); value) {
            buildPath = value;
            continue;
        }

        // Skip key-value options spanning two arguments.
        if (skipIrrelevantKeyValueArgument(argIndex, arg)) {
            continue;
        }

        // Skip rest of CMake options. This cover both valueless options like "-Wdev" and key-value options in
        // single arg form, like "-S=.." or "-S.."
        if (arg[0] == '-') {
            continue;
        }

        // Free lying arg is the source directory
        sourcePath = arg;
    }

    // Handle arguments that were not passed by user
    if (sourcePath.empty() && buildPath.empty()) {
        valid = false;
    }
    if (sourcePath.empty()) {
        sourcePath = ".";
    }
    if (buildPath.empty()) {
        buildPath = ".";
    }
}

const char *DumperArgumentParser::parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg,
                                                        const char *nextArg) {
    FATAL_ERROR_IF(prefix[0] != '-', "Prefix must start with a dash");
    const bool isLongArgName = prefix[1] == '-';
    FATAL_ERROR_IF(isLongArgName && prefix[2] == '-', "Prefix cannot have three dashes");

    // Early return if our prefix doesn't match
    if (currentArg.find(prefix) != 0) {
        return nullptr;
    }

    // Check for two args form, e.g. "cmake -S sourceDir"
    if (currentArg.length() == prefix.length()) {
        if (nextArg != nullptr) {
            argIndex++;
            return nextArg;
        } else {
            valid = false;
            return nullptr;
        }
    }

    // Check for one arg form with equal sign, e.g. "cmake -S=sourceDir"
    if (currentArg[prefix.length()] == '=') {
        // Equal sign form, value is in current arg, after the equals sign
        if (currentArg.length() > prefix.length() + 1) {
            return currentArg.substr(prefix.length() + 1).data();
        } else {
            valid = false;
            return nullptr;
        }
    }

    // Check for one arg form without equal sign, e.g. "cmake -SsourceDir"
    if (!isLongArgName) {
        return currentArg.substr(prefix.length()).data();
    }

    return nullptr;
}

bool DumperArgumentParser::skipIrrelevantKeyValueArgument(int &argIndex, std::string_view currentArg) {
    static const char *keyValueArgs[] = {
        "-G",
        "-P",
        "-C",
        "-U",
        "-G",
        "-T",
        "-A",
        "--toolchain",
        "--install-prefix",
        "--preset",
        "-P",
        "--find-package",
        "--graphviz",
        "--system-information",
        "--log-level",
        "--debug-find-pkg",
        "--debug-find-var",
        "--trace-format",
        "--trace-source",
        "--trace-redirect",
        "--profiling-format",
        "--profiling-output",
    };

    // We need to list them manually, because they change the meaning of the next argument.
    for (const char *keyValueArg : keyValueArgs) {
        // Check for two argument form
        if (currentArg == keyValueArg) {
            if (argIndex + 1 < argc) {
                argIndex++;
            } else {
                valid = false;
            }
            return true;
        }

        // Check for equals form
        const size_t keyValueArgLen = strlen(keyValueArg);
        if (currentArg.find(keyValueArg) == 0) {
            if (currentArg.length() == keyValueArgLen + 1) {
                if (currentArg[keyValueArgLen] == '=') {
                    valid = false;
                    return true;
                }
            }
        }
    }
    return false;
}

std::vector<std::string> DumperArgumentParser::constructArgsForCmake() const {
    std::vector<std::string> result = {};
    result.reserve(argc - 1);
    for (int i = cmakeArgsStartIndex; i < argc; i++) {
        result.emplace_back(argv[i]);
    }
    return result;
}

void DumperArgumentParser::printHelp() {
    const char *message = R"DELIMETER(Usage

    cmag [cmag_args] <cmake_command> <cmake_args>

Prepend name of cmag binary to CMake command to generate .cmag-project file. Cmag parses the
command line arguments to figure out values passed to CMake, like source directory or build
directory. All arguments are then passed verbatim to CMake. Output .cmag-project file will reside
in CMake's build directory.

Cmag supports a number of additional arguments, all of which must be specified before the CMake command.
    -v    show version of cmag.
    -p    name of the project. Used in filename before .cmag-project extension. Default is "project".
    -e    extra properties. By default Cmag dumps a predefined list of CMake properties. The user can extend this list
          by additional properties. Multiple properties are delimited by a semicolon.
    -d    json debug. Dump json files before generator expression evaluation. Useful for cmag development.
    -g    launch gui. Open generated project in cmag_browser immediately.
    -f    make find_package() global. By default IMPORTED targets are scoped only to the directory in which they are
          created. This causes cmag to be unable to gather all information about them. This option enables a CMake
          switch CMAKE_FIND_PACKAGE_TARGETS_GLOBAL, which make them all scoped globally. Use with caution - this can
          potentially break something in a project, which is not ready for it.

Examples:
    cmag cmake ..
    cmag /usr/bin/cmake ..
    cmag -p main_project cmake -S=. -B=build
    cmag -e "OUTPUT_NAME;LINK_FLAGS" cmake ..

)DELIMETER";
    printf("%s", message);
}
