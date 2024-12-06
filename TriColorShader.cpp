#include "TriColorShader.h"
#include "my_utils.h"

TriColorShader::TriColorShader(const GPoint pts[3], const GColor cols[3]) {
    memcpy(fPts, pts, sizeof(fPts));
    memcpy(fCols, cols, sizeof(fCols));

    GMatrix basis(
        fPts[1].x - fPts[0].x, fPts[2].x - fPts[0].x, fPts[0].x - 0,
        fPts[1].y - fPts[0].y, fPts[2].y - fPts[0].y, fPts[0].y - 0
    );

    // Compute the inverse area of the triangle
    float area = computeTriangleArea(fPts[0], fPts[1], fPts[2]);
    fAreaInv = (area != 0) ? (1.0f / area) : 0.0f;
}

bool TriColorShader::isOpaque() {
    return fCols[0].a == 1 && fCols[1].a == 1 && fCols[2].a == 1;
}

bool TriColorShader::setContext(const GMatrix& ctm) {
    GMatrix basis = compute_basis(fPts[0], fPts[1], fPts[2]);
    if (!GMatrix::Concat(ctm, basis).invert()) {
        return false;
    }
    fInverseCTM = ctm.invert().value();
    return true;
}

void TriColorShader::shadeRow(int x, int y, int count, GPixel row[]) {
    for (int i = 0; i < count; ++i) {
        GPoint p = {x + 0.5f + i, y + 0.5f};
        p = fInverseCTM *p;

        float w0 = fAreaInv * computeTriangleArea(fPts[1], fPts[2], p);
        float w1 = fAreaInv * computeTriangleArea(fPts[2], fPts[0], p);
        float w2 = fAreaInv * computeTriangleArea(fPts[0], fPts[1], p);

        GColor color = GColor::RGBA(
            fCols[0].r * w0 + fCols[1].r * w1 + fCols[2].r * w2,
            fCols[0].g * w0 + fCols[1].g * w1 + fCols[2].g * w2,
            fCols[0].b * w0 + fCols[1].b * w1 + fCols[2].b * w2,
            fCols[0].a * w0 + fCols[1].a * w1 + fCols[2].a * w2
        ).pinToUnit();

        row[i] = ColorToPixel(color);
    }
}


