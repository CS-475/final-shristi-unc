#ifndef TRI_COLOR_SHADER_H
#define TRI_COLOR_SHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GColor.h"

class TriColorShader : public GShader {
    GPoint fPts[3];  // Triangle vertices
    GColor fCols[3]; // Vertex colors
    float fAreaInv; // Inverse matrix for barycentric calculation
    GMatrix fInverseCTM;

public:
    TriColorShader(const GPoint pts[3], const GColor cols[3]);

    bool isOpaque() override;
    bool setContext(const GMatrix& ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;
};

#endif
