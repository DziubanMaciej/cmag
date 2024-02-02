#pragma once

#include <cmath>
#include <cstddef>

struct Vec {
    float x;
    float y;

    Vec operator-(const Vec &other) const {
        return Vec{x - other.x, y - other.y};
    }

    Vec operator+(const Vec &other) const {
        return Vec{x + other.x, y + other.y};
    }

    Vec rotated90() const {
        return Vec{y, -x};
    }

    Vec normalized() const {
        const float length = calculateLength();
        return Vec{x / length, y / length};
    }

    Vec scaled(float scale) const {
        return Vec{x * scale, y * scale};
    }

    float calculateLength() const {
        return sqrtf(x * x + y * y);
    }
};

bool isPointInsidePolygon(float pointX, float pointY, const float *polygon, size_t length);
bool isPointInsidePolygon(Vec point, const Vec *polygon, size_t vertexCountInPolygon);

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
