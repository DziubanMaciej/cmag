#pragma once

#include <cstdint>
#include <string>

union CmagVersion {
    struct {
        uint16_t major;
        uint16_t minor;
    } components;
    uint32_t raw;

    static constexpr CmagVersion fromComponents(uint16_t major, uint16_t minor) {
        CmagVersion version = {};
        version.components.major = major;
        version.components.minor = minor;
        return version;
    }

    std::string toString() const {
        return std::to_string(components.major) + "." + std::to_string(components.minor);
    }
};

constexpr static inline CmagVersion cmagVersion = CmagVersion::fromComponents(0, 1);
