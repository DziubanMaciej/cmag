#include "shapes.h"

const static float postcardVertices[] = {
    -0.5, -0.5, // v0
    +0.5, -0.5, // v1
    +1.0, +0.0, // v2
    +0.5, +0.5, // v3
    -0.5, +0.5, // v4
};
const ShapeInfo ShapeInfo::postcard(postcardVertices);

const static float squareVertices[] = {
    -1.0, -1.0, // v0
    +1.0, -1.0, // v1
    +1.0, +1.0, // v2
    -1.0, +1.0, // v3
};
const ShapeInfo ShapeInfo::square(squareVertices);
