/*
 *  Copyright 2024 Shristi
 */

#include "include/GMatrix.h"
#include <optional>
#include <cmath>
#include <cassert>


// Initialize the matrix to an identity matrix
GMatrix::GMatrix() {
    fMat[0] = 1; fMat[2] = 0; fMat[4] = 0;
    fMat[1] = 0; fMat[3] = 1; fMat[5] = 0;
}

// Translation matrix
GMatrix GMatrix::Translate(float tx, float ty) {
    return GMatrix(
        1, 0, tx, 
        0, 1, ty
    );
}

// Scaling matrix
GMatrix GMatrix::Scale(float sx, float sy) {
    return GMatrix(
        sx, 0, 0, 
        0, sy, 0);
}

// Rotation matrix (in radians)
GMatrix GMatrix::Rotate(float radians) {
    float cosVal = cos(radians);
    float sinVal = sin(radians);
    return GMatrix(
        cosVal, -sinVal, 0, 
        sinVal, cosVal, 0);
}


GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(
        a[0] * b[0] + a[2] * b[1],
        a[0] * b[2] + a[2] * b[3],
        a[0] * b[4] + a[2] * b[5] + a[4],
        a[1] * b[0] + a[3] * b[1],
        a[1] * b[2] + a[3] * b[3],
        a[1] * b[4] + a[3] * b[5] + a[5]
    );
}

// Matrix inversion
nonstd::optional<GMatrix> GMatrix::invert() const {
    float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];  // Correct determinant calculation
    if (det == 0) {
        return {};  // Return empty optional if matrix is not invertible
    }
    float invDet = 1.0f / det;

    return GMatrix(
        fMat[3] * invDet,              // a -> d / det
        -fMat[2] * invDet,             // c -> -c / det
        (fMat[2] * fMat[5] - fMat[3] * fMat[4]) * invDet,  // tx -> -(c * ty - d * tx) / det
        -fMat[1] * invDet,             // b -> -b / det
        fMat[0] * invDet,              // d -> a / det
        (fMat[1] * fMat[4] - fMat[0] * fMat[5]) * invDet   // ty -> -(b * tx - a * ty) / det
    );
}

// Map points using the matrix
// void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
//     for (int i = 0; i < count; ++i) {
//         dst[i].x = fMat[0] * src[i].x + fMat[2] * src[i].y + fMat[4];
//         dst[i].y = fMat[1] * src[i].x + fMat[3] * src[i].y + fMat[5];
//     }
// }

// Prof Reed's implementation
void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    const auto e0 = this->e0();
    const auto e1 = this->e1();
    const auto origin = this->origin();
    for (int i = 0; i < count; ++i) {
        dst[i] = e0 * src[i].x + e1 * src[i].y + origin;
    }
}