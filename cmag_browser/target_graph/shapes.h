#pragma once

#include "cmag_core/core/cmag_project.h"
#include "cmag_core/utils/error.h"

#include <array>
#include <cstdint>
#include <limits>

struct ShapeInfo {
    constexpr static inline uint32_t maxSubShapesCount = 3;
    constexpr static inline uint32_t maxVerticesCount = 240;
    constexpr static inline uint32_t floatsPerVertex = 2;

    struct SubShape {
        uint32_t vertexOffset = {};
        uint32_t vertexCount = {};
    };

    ShapeInfo(const float *floats, uint32_t floatsCount, std::array<uint32_t, maxSubShapesCount> subShapesOffsets = {});

    const float *floats;
    uint32_t floatsCount;
    uint32_t subShapesCount = 0;
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
