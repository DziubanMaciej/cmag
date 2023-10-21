#include "ArgumentParser.h"

#include <cstring>

static const char *keyValueArgs[] = {
    "-G",
    "-P",
    // TODO specify rest of them. See cmake -h
};

ArgumentParser::ArgumentParser(int argc, const char **argv) {
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
        extraArgs.emplace_back(std::string{"--graphviz="} + graphvizPath.string());
    }
}

const char *ArgumentParser::parseKeyValueArgument(const char *prefix, int &argIndex, const std::string &currentArg, const char *nextArg) {
    const auto prefixLen = strlen(prefix);

    if (currentArg.find(prefix) != 0) {
        return nullptr;
    }

    const char nextChar = currentArg.data()[prefixLen];
    if (nextChar == '\0') {
        // Two args form, value is in next arg
        if (nextArg) {
            argIndex++;
            return nextArg;
        } else {
            valid = false;
            return nullptr;
        }
    }

    if (nextChar == '=') {
        // Equal sign form, value is in current arg, after the equals sign
        if (currentArg.length() > prefixLen + 1) {
            return currentArg.c_str() + prefixLen + 1;
        } else {
            valid = false;
            return nullptr;
        }
    }

    return nullptr;
}