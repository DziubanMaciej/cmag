#pragma once

#include <string>
#include <string_view>
#include <vector>

inline std::vector<std::string_view> splitStringByChar(std::string_view value, bool ignoreSingleEntry, char separator) {
    if (value.empty()) {
        return {};
    }

    std::vector<std::string_view> result = {};

    size_t currentPosition = 0;
    size_t semicolonPosition = 0;
    while ((semicolonPosition = value.find(separator, currentPosition)) != std::string::npos) {
        std::string_view entry = std::string_view{value}.substr(currentPosition, semicolonPosition - currentPosition);
        result.push_back(entry);

        currentPosition = semicolonPosition + 1;
    }

    if (ignoreSingleEntry && result.empty()) {
        return {};
    }

    std::string_view entry = std::string_view{value}.substr(currentPosition);
    result.push_back(entry);

    return result;
}

inline std::vector<std::string_view> splitCmakeListString(std::string_view value, bool ignoreSingleEntry) {
    // If property value contains semicolons, most probably it's a list, because CMake delimits list entries with semicolons.
    // Split the value, so we can display it as a list of bullets in popup.
    return splitStringByChar(value, ignoreSingleEntry, ';');
}

inline std::string joinStringWithChar(const std::vector<std::string> &strings, char separator) {
    // Calculate required space for output string.
    size_t size = 1; // 1 because of null-termination
    for (const auto &string : strings) {
        if (!string.empty()) {
            size += string.size() + 1; // 1 because of separator
        }
    }

    // Prepare the string
    std::string result{};
    result.reserve(size);

    // Copy strings into the result
    bool separatorNeeded = false;
    for (const auto &string : strings) {
        if (string.empty()) {
            continue;
        }

        if (separatorNeeded) {
            result.push_back(separator);
        }

        result.append(string);

        separatorNeeded = true;
    }

    return result;
}

inline bool isValidCmakeTargetName(std::string_view targetName, bool allowDoubleColons) {
    // See https://cmake.org/cmake/help/latest/policy/CMP0037.html for validity rules
    // for target names in CMake.

    if (targetName.empty()) {
        return false;
    }

    for (char c : targetName) {
        // Docs: "Target names may contain upper and lower case letters, numbers, the underscore character (_), dot(.), plus(+) and minus(-)."
        if (isalnum(c) || c == '_' || c == '.' || c == '+' || c == '-') {
            continue;
        }

        // Docs: "As a special case, ALIAS and IMPORTED targets may contain two consecutive colons."
        // Actually any number of colons is allowed, even on the beginning or end of the name.
        if (c == ':' && allowDoubleColons) {
            continue;
        }

        return false;
    }

    return true;
}
