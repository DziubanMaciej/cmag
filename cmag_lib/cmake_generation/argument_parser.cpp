#include "argument_parser.h"

#include <cstring>

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

    int argIndex = 2; // skip our process name and cmake process name

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
    if (currentArg.find(prefix) != 0) {
        return nullptr;
    }

    const char nextChar = currentArg.data()[prefix.length()];
    if (nextChar == '\0') {
        // Two args form, value is in next arg
        if (nextArg != nullptr) {
            argIndex++;
            return nextArg;
        } else {
            valid = false;
            return nullptr;
        }
    }

    if (nextChar == '=') {
        // Equal sign form, value is in current arg, after the equals sign
        if (currentArg.length() > prefix.length() + 1) {
            return currentArg.substr(prefix.length() + 1).data();
        } else {
            valid = false;
            return nullptr;
        }
    }

    return nullptr;
}
std::vector<std::string> ArgumentParser::constructArgsForCmake() const {
    std::vector<std::string> result = {};
    result.reserve(argc - 1 + extraArgs.size());
    for (int i = 1; i < argc; i++) {
        result.emplace_back(argv[i]);
    }
    for (const auto &extraArg : extraArgs) {
        result.emplace_back(extraArg.c_str());
    }
    return result;
}
