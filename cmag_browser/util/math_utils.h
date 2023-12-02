#pragma once

#include <cstddef>

bool isPointInsidePolygon(float pointX, float pointY, const float *polygon, size_t length);

struct Vec {
    float x;
    float y;
};

struct Segment {
    Vec start;
    Vec end;
};

float length(Vec vec);
void scale(Vec &vec, float scale);

bool intersectSegments(
    Segment a,
    Segment b,
    float *outParameterA);

void trimSegment(
    Segment &segment,
    float parameterStart,
    float parameterEnd);

template <typename DataType>
inline DataType interpolate(DataType arg,
                            DataType minA, DataType maxA,
                            DataType minB, DataType maxB) {
    return (arg - minA) * (maxB - minB) / (maxA - minA) + minB;
}

template <typename DataType>
inline DataType interpolateDifference(DataType arg,
                                      DataType minA, DataType maxA,
                                      DataType minB, DataType maxB) {
    return arg * (maxB - minB) / (maxA - minA);
}

template <typename DataType>
inline DataType clamp(DataType arg, DataType min, DataType max) {
    if (arg < min) {
        return min;
    }
    if (arg > max) {
        return max;
    }
    return arg;
}
