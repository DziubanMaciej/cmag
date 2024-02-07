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

    Vec operator*(float scale) const {
        return Vec{x * scale, y * scale};
    }

    Vec rotated90() const {
        return Vec{y, -x};
    }

    Vec normalized() const {
        const float length = calculateLength();
        return Vec{x / length, y / length};
    }

    Vec scaled(float scale) const {
        return *this * scale;
    }

    float calculateLength() const {
        return sqrtf(x * x + y * y);
    }

    bool isInsidePolygon(const Vec *polygon, size_t vertexCountInPolygon) const;
};

static_assert(sizeof(Vec) == sizeof(float) * 2);
static_assert(sizeof(Vec[20]) == sizeof(float) * 40);

struct Segment {
    Vec start;
    Vec end;

    bool calculateIntersection(Segment other, float *outParameter) const;
    Segment trimed(float parameterStart, float parameterEnd) const;
};

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

constexpr static size_t maxValue(size_t bits) {
    return (1 << bits) - 1;
}

constexpr static size_t maxDecimalLength(size_t bits) {
    size_t val = maxValue(bits);
    size_t decimalLength = 1;
    while(val >= 10) {
        decimalLength++;
        val /= 10;
    }
    return decimalLength;
}
