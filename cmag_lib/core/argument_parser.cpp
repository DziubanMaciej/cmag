#include "argument_parser.h"

#include <cstring>
#include "cmag_lib/utils/error.h"

static const char *keyValueArgs[] = {
    "-G",
    "-P",
    // TODO specify rest of them. See cmake -h
};

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

        // Handle arguments that we care about
        if (const char *value = parseKeyValueArgument("-S", argIndex, arg, nextArg); value) {
            sourcePath = value;
        }
        if (const char *value = parseKeyValueArgument("-B", argIndex, arg, nextArg); value) {
            buildPath = value;
        }
        if (const char *value = parseKeyValueArgument("--graphviz", argIndex, arg, nextArg); value) {
            graphvizPath = value;
        }

        // Skip key-value options spanning two arguments
        for (const char *keyValueArg : keyValueArgs) {
            if (arg == keyValueArg) {
                argIndex++;
                continue;
            }
        }

        // Skip rest of CMake options. This cover both valueless options like "-Wdev" and key-value options with equal sign form, like "-S=.."
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
    if (sourcePath.empty()) {
        valid = false;
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

const char *ArgumentParser::parseKeyValueArgument(std::string_view prefix, int &argIndex, std::string_view currentArg, const char *nextArg) {
    FATAL_ERROR_IF(prefix[0] != '-', "Prefix must start with a dash")
    const bool isLongArgName = prefix[1] == '-';
    FATAL_ERROR_IF(isLongArgName && prefix[2] == '-', "Prefix cannot have three dashes")

    // Early return if our prefix doesn't match
    if (currentArg.find(prefix) != 0) {
        return nullptr;
    }

    // Check for two args form, e.g. "cmake -S sourceDir"
    const char nextChar = currentArg[prefix.length()];
    if (nextChar == '\0') {
        if (nextArg != nullptr) {
            argIndex++;
            return nextArg;
        } else {
            valid = false;
            return nullptr;
        }
    }

    // Check for one arg form with equal sign, e.g. "cmake -S=sourceDir"
    if (nextChar == '=') {
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
