#pragma once

bool isPointInsidePolygon(float pointX, float pointY, const float *polygon, size_t length);

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
