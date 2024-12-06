/*
 *  Copyright 2024 Shristi
 */


#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "include/GPaint.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include <stack>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device) {
        fMatrixStack.push(GMatrix());  // Initialize with identity matrix
    }

    void save() override;
    void restore() override;
    void concat(const GMatrix& matrix) override;

    void clear(const GColor& color) override;
    void fillRectX(const GRect& rect, const GColor& color);
    void drawRect(const GRect& rect, const GPaint& paint) override;
    void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override;
    void drawPath(const GPath& path, const GPaint& paint) override;  // For non-convex polygons
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override;
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override;
    // void drawTriangleWithTex(const GPoint pts[3], const GPoint tex[3], GShader* originalShader);
private:
    const GBitmap fDevice;
    std::stack<GMatrix> fMatrixStack;  // Stack of transformation matrices
};

#endif
