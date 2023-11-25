#pragma once

#include "cmag_lib/core/cmag_project.h"

#include <cstddef>
#include <limits>

struct ShapeInfo {
    template <size_t verticesCount>
    ShapeInfo(const float (&vertices)[verticesCount]) : vertices(vertices),
                                                        verticesCount(verticesCount) {
        static_assert(verticesCount > 2);
        static_assert(verticesCount % 2 == 0);

        for (size_t i = 0; i < verticesCount; i += 2) {
            bounds.minX = std::min(bounds.minX, vertices[i]);
            bounds.maxX = std::max(bounds.maxX, vertices[i]);
            bounds.minY = std::min(bounds.minY, vertices[i + 1]);
            bounds.maxY = std::max(bounds.maxY, vertices[i + 1]);
        }
    }

    const float *vertices;
    size_t verticesCount;
    struct {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
    } bounds;

    const static ShapeInfo postcard;
    const static ShapeInfo square;
};
