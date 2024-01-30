#pragma once

#include <type_traits>

#define BITFIELD_ENUM(EnumT)                                                                  \
    inline EnumT operator&(EnumT a, EnumT b) {                                                \
        using UnderlyingT = std::underlying_type_t<EnumT>;                                    \
        return static_cast<EnumT>(static_cast<UnderlyingT>(a) & static_cast<UnderlyingT>(b)); \
    }                                                                                         \
                                                                                              \
    inline EnumT operator|(EnumT a, EnumT b) {                                                \
        using UnderlyingT = std::underlying_type_t<EnumT>;                                    \
        return static_cast<EnumT>(static_cast<UnderlyingT>(a) | static_cast<UnderlyingT>(b)); \
    }                                                                                         \
                                                                                              \
    inline bool has##EnumT##Bit(EnumT bitfield, EnumT mask) {                                 \
        using UnderlyingT = std::underlying_type_t<EnumT>;                                    \
        return (static_cast<UnderlyingT>(bitfield) & static_cast<UnderlyingT>(mask)) != 0;    \
    }
