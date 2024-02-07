#pragma once

#include "cmag_core/utils/error.h"
#include "cmag_core/utils/math_utils.h"

#include <cstdint>
#include <optional>
#include <string>

struct CmagVersion {
    constexpr static inline size_t comp0bits = 8;
    constexpr static inline size_t comp1bits = 16;
    constexpr static inline size_t comp2bits = 8;
    static_assert(comp0bits + comp1bits + comp2bits == 32);

    uint32_t comp0 : comp0bits;
    uint32_t comp1 : comp1bits;
    uint32_t comp2 : comp2bits;

    template <uint32_t comp0, uint32_t comp1, uint32_t comp2>
    static constexpr CmagVersion fromComponents() {
        static_assert(comp0 <= maxValue(comp0bits), "Invalid comp0 value");
        static_assert(comp1 <= maxValue(comp1bits), "Invalid comp1 value");
        static_assert(comp2 <= maxValue(comp2bits), "Invalid comp2 value");
        return CmagVersion{
            comp0,
            comp1,
            comp2,
        };
    }

    static std::optional<CmagVersion> fromComponents(uint32_t comp0, uint32_t comp1, uint32_t comp2) {
        if (comp0 > maxValue(comp0bits) ||
            comp1 > maxValue(comp1bits) ||
            comp2 > maxValue(comp2bits)) {
            return {};
        }
        return CmagVersion{
            comp0,
            comp1,
            comp2,
        };
    }

    static std::optional<CmagVersion> fromString(const std::string &arg) {
        uint32_t compValue0{};
        uint32_t compValue1{};
        uint32_t compValue2{};

        const int result = sscanf(arg.c_str(), "%u.%u.%u", &compValue0, &compValue1, &compValue2);
        if (result != 3) {
            return {};
        }
        return fromComponents(compValue0, compValue1, compValue2);
    }

    std::string toString() const {
        constexpr static size_t maxStringLength =
            maxDecimalLength(comp0bits) +
            maxDecimalLength(comp1bits) +
            maxDecimalLength(comp2bits) +
            3; // two dots and zero termination

        char buffer[maxStringLength];
        snprintf(buffer, maxStringLength, "%u.%u.%u", comp0, comp1, comp2);

        return std::string{buffer};
    }

    bool operator==(const CmagVersion &other) const {
        return (comp0 == other.comp0) && (comp1 == other.comp1) && (comp2 == other.comp2);
    }

    bool operator!=(const CmagVersion &other) const {
        return (comp0 != other.comp0) || (comp1 != other.comp1) || (comp2 != other.comp2);
    }

    bool operator<(const CmagVersion &other) const {
        if (comp0 < other.comp0) {
            return true;
        } else if (comp0 > other.comp0) {
            return false;
        }
        if (comp1 < other.comp1) {
            return true;
        } else if (comp1 > other.comp1) {
            return false;
        }
        return comp2 < other.comp2;
    }
};

constexpr static inline CmagVersion cmagVersion = CmagVersion::fromComponents<0, 1, 0>();
