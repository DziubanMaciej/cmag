#pragma once

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/utils/error.h"

#include <array>
#include <cstddef>
#include <limits>

struct ShapeInfo {
    constexpr static inline size_t maxSubShapesCount = 3;
    constexpr static inline size_t maxVerticesCount = 240;

    struct SubShape {
        size_t offset = {};
        size_t count = {};
    };

    template <size_t verticesCount>
    ShapeInfo(const float (&vertices)[verticesCount], std::array<size_t, maxSubShapesCount> subShapesOffsets = {})
        : vertices(vertices),
          verticesCount(verticesCount) {
        static_assert(verticesCount > 2);
        static_assert(verticesCount % 2 == 0);

        for (size_t subShapeIndex = 0; subShapeIndex < maxSubShapesCount; subShapeIndex++) {
            const size_t currentOffset = subShapesOffsets[subShapeIndex];

            if (subShapeIndex == 0) {
                FATAL_ERROR_IF(currentOffset != 0, "First subshape offset should be 0");
                subShapesCount++;
                continue;
            }

            SubShape &currentSubShape = subShapes[subShapeIndex];
            SubShape &previousSubShape = subShapes[subShapeIndex - 1];
            if (currentOffset > previousSubShape.offset) {
                previousSubShape.count = currentOffset - previousSubShape.offset;
                currentSubShape.offset = currentOffset;
                subShapesCount++;
            } else {
                break;
            }
        }

        SubShape &lastSubShape = subShapes[subShapesCount - 1];
        lastSubShape.count = verticesCount - lastSubShape.offset;

        for (size_t i = 0; i < verticesCount; i += 2) {
            bounds.minX = std::min(bounds.minX, vertices[i]);
            bounds.maxX = std::max(bounds.maxX, vertices[i]);
            bounds.minY = std::min(bounds.minY, vertices[i + 1]);
            bounds.maxY = std::max(bounds.maxY, vertices[i + 1]);
        }
    }

    const float *vertices;
    size_t verticesCount;
    size_t subShapesCount = 0;

    SubShape subShapes[maxSubShapesCount] = {};

    struct {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::min();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::min();
    } bounds;

    const static ShapeInfo staticLib;
    const static ShapeInfo executable;
    const static ShapeInfo sharedLib;
    const static ShapeInfo moduleLib;
    const static ShapeInfo customTarget;
    const static ShapeInfo interfaceLib;
    const static ShapeInfo objectLib;
    const static ShapeInfo unknownLib;
};
