#include "cmag_lib/utils/error.h"

struct Point {
    float x, y;
};

float crossProduct(const Point &p0, const Point &p1, const Point &p2) {
    return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
}

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
