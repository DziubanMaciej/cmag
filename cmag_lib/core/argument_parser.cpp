#include "argument_parser.h"

#include "cmag_lib/utils/error.h"

#include <cstring>

ArgumentParser::ArgumentParser(int argc, const char **argv) : argc(argc), argv(argv) {
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
        if (arg == "-d") {
            jsonDebug = true;
            validArg = true;
        }
        if (arg == "-g") {
            launchGui = true;
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
        if (const char *value = parseKeyValueArgument("--graphviz", argIndex, arg, nextArg); value) {
            graphvizPath = value;
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
    if (projectName.empty()) {
        projectName = "project";
    }
    if (sourcePath.empty() && buildPath.empty()) {
        valid = false;
    }
    if (sourcePath.empty()) {
        sourcePath = ".";
    }
    if (buildPath.empty()) {
        buildPath = ".";
    }
    if (graphvizPath.empty()) {
        graphvizPath = buildPath / "graph.dot";
        extraArgs.emplace_back("--graphviz");
        extraArgs.emplace_back(graphvizPath.string());
    }
}

const char *ArgumentParser::parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg,
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

bool ArgumentParser::skipIrrelevantKeyValueArgument(int &argIndex, std::string_view currentArg) {
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
        if (currentArg == keyValueArg) {
            argIndex++;
            return true;
        }
    }
    return false;
}


std::vector<std::string> ArgumentParser::constructArgsForCmake() const {
    std::vector<std::string> result = {};
    result.reserve(argc - 1 + extraArgs.size());
    for (int i = cmakeArgsStartIndex; i < argc; i++) {
        result.emplace_back(argv[i]);
    }
    for (const auto &extraArg : extraArgs) {
        result.emplace_back(extraArg.c_str());
    }
    return result;
}

void ArgumentParser::printHelp() {
    const char *message = R"DELIMETER(Usage

    cmag [cmag_args] <cmake_command> <cmake_args>

Prepend name of cmag binary to CMake command to generate .cmag-project file. Cmag parses the
command line arguments to figure out values passed to CMake, like source directory or build
directory. All arguments are then passed verbatim to CMake. Output .cmag-project file will reside
in CMake's build directory.

Cmag supports a number of additional arguments, all of which must be specified before the CMake command.
    -p    name of the project. Used in filename before .cmag-project extension. Default is "project".
    -e    extra properties. By default Cmag dumps a predefined list of CMake properties. The user can extend this list
          by additional properties. Multiple properties are delimited by a semicolon.
    -d    json debug. Dump json files before generator expression evaluation. Useful for cmag development.
    -g    launch gui. Open generated project in cmag_browser immediately.

Examples:
    cmag cmake ..
    cmag /usr/bin/cmake ..
    cmag -p main_project cmake -S=. -B=build
    cmag -e "OUTPUT_NAME;LINK_FLAGS" cmake ..

)DELIMETER";
    printf("%s", message);
}
