#include "math_utils.h"

#include "cmag_core/utils/error.h"

#include <cmath>

struct Point {
    float x, y;
};

float crossProduct(const Point &p0, const Point &p1, const Point &p2) {
    return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

// TODO remove this copy-pasted overload
bool isPointInsidePolygon(float pointX, float pointY, const float *polygon, size_t length) {
    FATAL_ERROR_IF(length < 6, "Invalid polygon, length=", length);
    FATAL_ERROR_IF(length % 2 == 1, "Invalid polygon, length=", length);

    int windingNumber = 0;

    Point point{pointX, pointY};

    for (size_t currentVertexIndex = 0; currentVertexIndex < length; currentVertexIndex += 2) {
        const size_t nextVertexIndex = (currentVertexIndex + 2) % length;
        Point currentVertex = Point{polygon[currentVertexIndex], polygon[currentVertexIndex + 1]};
        Point nextVertex = Point{polygon[nextVertexIndex], polygon[nextVertexIndex + 1]};

        if (currentVertex.y <= point.y) {
            if (nextVertex.y > point.y && crossProduct(currentVertex, nextVertex, point) > 0) {
                ++windingNumber;
            }
        } else {
            if (nextVertex.y <= point.y && crossProduct(currentVertex, nextVertex, point) < 0) {
                --windingNumber;
            }
        }
    }

    return (windingNumber != 0);
}

bool isPointInsidePolygon(Vec point, const Vec *polygon, size_t vertexCountInPolygon) {
    FATAL_ERROR_IF(vertexCountInPolygon < 3, "Invalid polygon, length=", vertexCountInPolygon);

    auto crossProduct = [](const Vec &p0, const Vec &p1, const Vec &p2) {
        return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
    };

    int windingNumber = 0;

    for (size_t currentVertexIndex = 0; currentVertexIndex < vertexCountInPolygon; currentVertexIndex++) {
        const size_t nextVertexIndex = (currentVertexIndex + 1) % vertexCountInPolygon;
        Vec currentVertex = polygon[currentVertexIndex];
        Vec nextVertex = polygon[nextVertexIndex];

        if (currentVertex.y <= point.y) {
            if (nextVertex.y > point.y && crossProduct(currentVertex, nextVertex, point) > 0) {
                ++windingNumber;
            }
        } else {
            if (nextVertex.y <= point.y && crossProduct(currentVertex, nextVertex, point) < 0) {
                --windingNumber;
            }
        }
    }

    return (windingNumber != 0);
}

float length(Vec vec) {
    return sqrtf(vec.x * vec.x + vec.y * vec.y);
}
void scale(Vec &vec, float scale) {
    vec.x *= scale;
    vec.y *= scale;
}

bool intersectSegments(
    Segment a,
    Segment b,
    float *outParameterA) {

    auto cross = [](Vec left, Vec right) {
        return left.x * right.y - left.y * right.x;
    };

    const Vec directionA = {
        a.end.x - a.start.x,
        a.end.y - a.start.y,
    };
    const Vec directionB = {
        b.end.x - b.start.x,
        b.end.y - b.start.y,
    };

    // Early return if segments are collinear. Actually there are three situations that could occur here:
    // 1. One vertex intersection
    // 2. No intersection
    // 3. Infinite vertices intersection
    // Since we need to return a parameter for segment 'a', only situation 2. would be meaningful. However,
    // this function is used to intersect with a polygon, so if we can skip it and we'll find intersection
    // with some other, non-parallel side.
    const float directionsCross = cross(directionA, directionB);
    if (directionsCross == 0) {
        return false;
    }

    // Calculate parameters for segments 'a' and 'b'.
    const Vec originsDiff = {b.start.x - a.start.x, b.start.y - a.start.y};
    const float parameterA = cross(originsDiff, directionB) / directionsCross;
    const float parameterB = cross(originsDiff, directionA) / directionsCross;

    // Both parameters must be between 0 and 1 for segments to intersect.
    if (0 > parameterA || parameterA > 1 || 0 > parameterB || parameterB > 1) {
        return false;
    }

    // Our segments intersect
    *outParameterA = parameterA;
    return true;
}

void trimSegment(
    Segment &segment,
    float parameterStart,
    float parameterEnd) {

    const Vec diff = {
        segment.end.x - segment.start.x,
        segment.end.y - segment.start.y,
    };

    segment.end = Vec{
        segment.start.x + diff.x * parameterEnd,
        segment.start.y + diff.y * parameterEnd,
    };
    segment.start = Vec{
        segment.start.x + diff.x * parameterStart,
        segment.start.y + diff.y * parameterStart,
    };
}