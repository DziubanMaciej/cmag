#pragma once

#include <string>
#include <string_view>
#include <vector>

inline std::vector<std::string_view> splitCmakeListString(std::string_view value, bool ignoreSingleEntry) {
    // If property value contains semicolons, most probably it's a list, because CMake delimits list entries with semicolons.
    // Split the value, so we can display it as a list of bullets in popup.

    if (value.empty()) {
        return {};
    }

    std::vector<std::string_view> result = {};

    size_t currentPosition = 0;
    size_t semicolonPosition = 0;
    while ((semicolonPosition = value.find(';', currentPosition)) != std::string::npos) {
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
