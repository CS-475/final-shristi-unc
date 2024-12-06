/*
 *  Copyright 2024 Shristi
 */

#ifndef _MY_UTILS_H_
#define _MY_UTILS_H_

#define GET_ALPHA(p)  ((p >> GPIXEL_SHIFT_A) & 0xFF)
#define GET_RED(p)    ((p >> GPIXEL_SHIFT_R) & 0xFF)
#define GET_GREEN(p)  ((p >> GPIXEL_SHIFT_G) & 0xFF)
#define GET_BLUE(p)   ((p >> GPIXEL_SHIFT_B) & 0xFF)

#define PACK_ARGB(a, r, g, b)  GPixel_PackARGB(a, r, g, b)

#include "include/GColor.h"
#include "include/GPixel.h"
#include "include/GMath.h"
#include "include/GPaint.h"
#include "include/GPoint.h"
#include "include/GBlendMode.h"
#include "include/GRect.h"
#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

using BlendFunc = GPixel(GPixel, GPixel);

// Represents a line segment used for filling the polygon
struct Edge {
    int top, bottom; // The y-coordinates defining the start (top) and end (bottom) of the edge
    int winding;    // Winding value (+1 for up, -1 for down)
    float slope;
    float b;

    Edge(float x0, float x1, float y0, float y1, int winding)
        : winding(winding) {
            slope = (x1 - x0) / (y1 - y0);
            top = GRoundToInt(y0);
            bottom = GRoundToInt(y1);
            b = x0 - slope * y0;
        }

    // Compute the X coordinate at a specific Y value (used for scan conversion)
    float computeX(int y) const {
        return slope * (y + 0.5f) + b;
    }

    // Check if the edge is valid at a given Y coordinate
    bool isValid(int y) const {
        return y >= top && y < bottom;
    }
};

// Comparator function used for sorting edges based on top value (y-coordinate of the edge’s starting point) so edges are processed top to bottom
inline bool compareEdges(const Edge& a, const Edge& b) {
    // return a.top < b.top;
    if (a.top != b.top) {
        return a.top < b.top;
    }
    // return a.computeX(a.top) < a.computeX(b.top); // Compare based on initial X coordinate
    return a.slope < b.slope; // Compare based on the slope or initial x-values
}

inline void addEdge(std::vector<Edge>& edges, const GPoint& p0, const GPoint& p1) {
    if (p0.y == p1.y) {
        
        return;  // Ignore horizontal edges?
    }

    float x0, x1, top, bottom;
    int winding;
    
    if (p0.y < p1.y) {
        // p0 is the top point
        x0 = p0.x;
        x1 = p1.x;
        top = p0.y;
        bottom = p1.y;
        winding = +1;  // Up edge
    } else {
        // p1 is the top point
        x0 = p1.x;
        x1 = p0.x;
        top = p1.y;
        bottom = p0.y;
        winding = -1;  // Down edge
    }

    Edge e = Edge(x0, x1, top, bottom, winding);
    if (e.top != e.bottom){
        edges.push_back(e);
    }
    
}

// Helper function to avoid multiple divisions by 255, while ensuring R, G, B <= A
inline int divide255(int value) {
    return (value + 128) * 257 >> 16;  // Fast approximation of division by 255
}

// Helper function to convert GColor to GPixel
inline GPixel ColorToPixel(const GColor& color) {
    // Clamp color values between 0 and 1
    float a = GPinToUnit(color.a);
    float r = GPinToUnit(color.r) * a;
    float g = GPinToUnit(color.g) * a;
    float b = GPinToUnit(color.b) * a;

    return GPixel_PackARGB(
        GRoundToInt(a * 255),
        GRoundToInt(r * 255),
        GRoundToInt(g * 255),
        GRoundToInt(b * 255)
    );
}

// Helper function to check if a point is inside a convex polygon
inline bool pointInPolygon(const GPoint& p, const GPoint points[], int count) {
    bool inside = false;
    for (int i = 0, j = count - 1; i < count; j = i++) {
        if (((points[i].y > p.y) != (points[j].y > p.y)) &&
            (p.x < (points[j].x - points[i].x) * (p.y - points[i].y) / (points[j].y - points[i].y) + points[i].x)) {
            inside = !inside;
        }
    }
    return inside;
}

// Helper function to compute the bounding box of a convex polygon
inline GRect computeBounds(const GPoint points[], int count) {
    float left = points[0].x, top = points[0].y, right = points[0].x, bottom = points[0].y;
    for (int i = 1; i < count; ++i) {
        left = std::min(left, points[i].x);
        top = std::min(top, points[i].y);
        right = std::max(right, points[i].x);
        bottom = std::max(bottom, points[i].y);
    }
    return GRect::LTRB(left, top, right, bottom);
}


inline GPixel blend_clear(GPixel src, GPixel dst){
    return 0;
}
inline GPixel blend_src(GPixel src, GPixel dst) {
    return src;
}
inline GPixel blend_dst(GPixel src, GPixel dst) {
    return dst;
}

// kSrcOver (S + (1 - Sa)D): Source over destination
inline GPixel blend_srcOver(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);
    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invSa = 255 - Sa;  // (1 - Sa)

    int A = Sa + divide255(Da * invSa);
    int R = Sr + divide255(Dr * invSa);
    int G = Sg + divide255(Dg * invSa);
    int B = Sb + divide255(Db * invSa);

    return PACK_ARGB(A, R, G, B);
}

// kDstOver (D + (1 - Da)S): Destination over source
inline GPixel blend_dstOver(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);
    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invDa = 255 - Da;  // (1 - Da)

    int A = Da + divide255(Sa * invDa);
    int R = Dr + divide255(Sr * invDa);
    int G = Dg + divide255(Sg * invDa);
    int B = Db + divide255(Sb * invDa);

    return PACK_ARGB(A, R, G, B);
}

// kSrcIn (Da * S): Source where destination exists
inline GPixel blend_srcIn(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);

    int A = divide255(Sa * Da);
    int R = divide255(Sr * Da);
    int G = divide255(Sg * Da);
    int B = divide255(Sb * Da);

    return PACK_ARGB(A, R, G, B);
}

// kDstIn (Sa * D): Destination where source exists
inline GPixel blend_dstIn(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int A = divide255(Da * Sa);
    int R = divide255(Dr * Sa);
    int G = divide255(Dg * Sa);
    int B = divide255(Db * Sa);

    return PACK_ARGB(A, R, G, B);
}

// kSrcOut ((1 - Da) * S): Source outside the destination
inline GPixel blend_srcOut(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);

    int invDa = 255 - Da;  // (1 - Da)

    int A = divide255(Sa * invDa);
    int R = divide255(Sr * invDa);
    int G = divide255(Sg * invDa);
    int B = divide255(Sb * invDa);

    return PACK_ARGB(A, R, G, B);
}

// kDstOut ((1 - Sa) * D): Destination outside the source
inline GPixel blend_dstOut(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invSa = 255 - Sa;  // (1 - Sa)

    int A = divide255(Da * invSa);
    int R = divide255(Dr * invSa);
    int G = divide255(Dg * invSa);
    int B = divide255(Db * invSa);

    return PACK_ARGB(A, R, G, B);
}

// kSrcATop (Da * S + (1 - Sa) * D): Source atop destination
inline GPixel blend_srcATop(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);
    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invSa = 255 - Sa;  // (1 - Sa)

    int A = divide255((Sa * Da) + (Da * invSa ));
    int R = divide255((Sr * Da ) + (Dr * invSa));
    int G = divide255((Sg * Da ) + (Dg * invSa));
    int B = divide255((Sb * Da ) + (Db * invSa));

    return PACK_ARGB(A, R, G, B);
}

// kDstATop (Sa * D + (1 - Da) * S): Destination atop source
inline GPixel blend_dstATop(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);
    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invDa = 255 - Da;  // (1 - Da)

    int A = divide255((Sa * Da) + (Sa * invDa));
    int R = divide255((Dr * Sa) + (Sr * invDa));
    int G = divide255((Dg * Sa) + (Sg * invDa));
    int B = divide255((Db * Sa) + (Sb * invDa));

    return PACK_ARGB(A, R, G, B);
}

// kXor ((1 - Sa) * D + (1 - Da) * S): Exclusive or blend
inline GPixel blend_xor(GPixel src, GPixel dst) {
    int Sa = GET_ALPHA(src);
    int Da = GET_ALPHA(dst);

    int Sr = GET_RED(src), Sg = GET_GREEN(src), Sb = GET_BLUE(src);
    int Dr = GET_RED(dst), Dg = GET_GREEN(dst), Db = GET_BLUE(dst);

    int invSa = 255 - Sa;  // (1 - Sa)
    int invDa = 255 - Da;  // (1 - Da)

    int A = divide255((Da * invSa) + (Sa * invDa));
    int R = divide255((Dr * invSa) + (Sr * invDa));
    int G = divide255((Dg * invSa) + (Sg * invDa));
    int B = divide255((Db * invSa) + (Sb * invDa));

    return PACK_ARGB(A, R, G, B);
}

inline GBlendMode optimize(GBlendMode mode, float alpha) {
    
    // 100% Opaque (alpha == 1.0)
    if (alpha == 1.0f) {
        // printf("Optimizing because opaque!");
        if (mode == GBlendMode::kSrcOver) {
            return GBlendMode::kSrc;  // No need for SrcOver when source is fully opaque
        } 
        if (mode == GBlendMode::kSrcIn) {
            return GBlendMode::kSrc;  // SrcIn becomes Src if source is fully opaque
        } 
        if (mode == GBlendMode::kDstIn) {
            return GBlendMode::kDst;  // DstIn becomes Dst if source is fully opaque
        } 
        
        if (mode == GBlendMode::kDstOver) {
            return GBlendMode::kDst;  // DstOver becomes Dst if source is fully opaque
        } 
        if (mode == GBlendMode::kSrcOut) {
            return GBlendMode::kClear; // SrcOut becomes Clear if source is fully opaque
        } 
        if (mode == GBlendMode::kSrcATop) {
            return GBlendMode::kDst;  // SrcATop becomes Dst if source is fully opaque
        } 
        if (mode == GBlendMode::kXor) {
            return GBlendMode::kDstOut;  // XOR becomes DstOut if source is fully opaque
        } 
        
    }

    // 100% Transparent (alpha == 0.0)
    if (alpha == 0.0f) {
        // printf("Optimizing because transparent!");
        if (mode == GBlendMode::kSrcIn) {
            return GBlendMode::kClear;  // SrcIn becomes Clear if source is fully transparent
        } 
        if (mode == GBlendMode::kDstOut) {
            return GBlendMode::kClear;  // DstOut becomes Clear if source is fully transparent
        } 
        if (mode == GBlendMode::kSrcOver) {
            return GBlendMode::kDst;    // SrcOver with transparent source becomes Dst
        } 
        if (mode == GBlendMode::kSrcATop) {
            return GBlendMode::kDst;    // SrcATop with transparent source becomes Dst
        } 
        if (mode == GBlendMode::kDstIn) {
            return GBlendMode::kClear;  // DstIn becomes Clear if source is fully transparent
        } 
        if (mode == GBlendMode::kXor) {
            return GBlendMode::kDst;    // XOR with transparent source becomes Dst
        } 
        if (mode == GBlendMode::kSrcOut) {
            return GBlendMode::kClear;  // SrcOut with transparent source becomes Clear
        } 
    }

    return mode; // No optimization applicable
}

inline BlendFunc* get_func(GBlendMode blendMode){
    switch (blendMode) {
        case GBlendMode::kClear:
            return blend_clear;
        case GBlendMode::kSrc:
            return blend_src;
        case GBlendMode::kDst:
            return blend_dst;
        case GBlendMode::kSrcOver:
            return blend_srcOver;
        case GBlendMode::kDstOver:
            return blend_dstOver;
        case GBlendMode::kSrcIn:
            return blend_srcIn;
        case GBlendMode::kDstIn:
            return blend_dstIn;
        case GBlendMode::kSrcOut:
            return blend_srcOut;
        case GBlendMode::kDstOut:
            return blend_dstOut;
        case GBlendMode::kSrcATop:
            return blend_srcATop;
        case GBlendMode::kDstATop:
            return blend_dstATop;
        case GBlendMode::kXor:
            return blend_xor;
    }
}

inline void blit(float leftX, int y, float width, const GPaint& paint, const GBitmap& fDevice, const GMatrix& ctm) {
    // Log the values of leftX, width, and y
    // std::cout << "blit called with: leftX = " << leftX << ", width = " << width << ", y = " << y << std::endl;

    // Ensure y is within valid bounds
    if (y < 0 || y >= fDevice.height()) {
        // std::cout << "Skipping blit because y is out of bounds: " << y << std::endl;
        return;  // Skip drawing if y is out of bounds
    }
    
    // Ensure width is valid (i.e., positive)
    int intWidth = GRoundToInt(width);
    if (intWidth <= 0) {
        // std::cout << "Skipping blit because intWidth is invalid: " << intWidth << std::endl;
        return;  // Skip if the width is zero or negative
    }

    // Ensure leftX is within valid bounds
    int intLeftX = GRoundToInt(leftX);
    if (intLeftX >= fDevice.width()) {
        // std::cout << "Skipping blit because intLeftX is out of bounds: " << intLeftX << std::endl;
        return;  // Skip if the starting X is out of bounds
    }

    // Adjust if leftX is negative to prevent accessing out-of-bounds pixels
    if (intLeftX < 0) {
        // std::cout << "Adjusting leftX because it is negative: " << intLeftX << std::endl;
        intWidth += intLeftX;  // Adjust the width to account for the negative offset
        intLeftX = 0;
    }

    // Bound the width so we don't overrun the right edge
    if (intLeftX + intWidth > fDevice.width()) {
        // std::cout << "Adjusting intWidth because it exceeds the right edge" << std::endl;
        intWidth = fDevice.width() - intLeftX;
    }

    // Ensure intWidth is still valid after all adjustments
    if (intWidth <= 0) {
        // std::cout << "Skipping blit because intWidth is now invalid after adjustments: " << intWidth << std::endl;
        return;  // Skip if width becomes non-positive after adjustments
    }

    // std::cout << "Final values before shading: leftX = " << intLeftX << ", width = " << intWidth << ", y = " << y << std::endl;

    GShader* shader = paint.peekShader();
    GBlendMode blendMode = paint.getBlendMode();
    BlendFunc* blender = get_func(blendMode);

    if (shader && shader->setContext(ctm)) {
        // Allocate row buffer based on calculated width
        GPixel row[intWidth];
        shader->shadeRow(intLeftX, y, intWidth, row);

        // Get the destination row and apply the pixels with blending
        GPixel* row_addr = fDevice.getAddr(0, y);
        for (int i = 0; i < intWidth; ++i) {
            int x = intLeftX + i;
            if (x >= 0 && x < fDevice.width()) {  // Ensure x is within bounds
                GPixel dstPixel = row_addr[x];  // Get destination pixel
                GPixel srcPixel = row[i];       // Get source pixel from shader
                row_addr[x] = blender(srcPixel, dstPixel);  // Apply blend mode
            }
        }
    } else {
        // Fallback: No shader, use solid color
        GPixel srcPixel = ColorToPixel(paint.getColor());

        // Apply the solid color with blending
        GPixel* row_addr = fDevice.getAddr(0, y);
        for (int i = 0; i < intWidth; ++i) {
            int x = intLeftX + i;
            if (x >= 0 && x < fDevice.width()) {  // Ensure x is within bounds
                GPixel dstPixel = row_addr[x];  // Get destination pixel
                row_addr[x] = blender(srcPixel, dstPixel);  // Apply blend mode
            }
        }
    }

    // std::cout << "blit finished successfully for y = " << y << std::endl;
}



// Helper function to solve quadratic equation ax^2 + bx + c = 0
inline std::vector<float> solveQuadratic(float a, float b, float c) {
    std::vector<float> roots;

    if (a == 0) {
        // Linear case: bx + c = 0
        if (b != 0) {
            float t = -c / b;
            // printf("t = %f\n",t);
            if (t >= 0 && t <= 1) roots.push_back(t);
        }
    } else {
        // Quadratic case
        float discriminant = b * b - 4 * a * c;

        if (discriminant >= 0) {
            float sqrtD = sqrt(discriminant);
            float t1 = (-b + sqrtD) / (2 * a);
            float t2 = (-b - sqrtD) / (2 * a);

            if (t1 >= 0 && t1 <= 1) roots.push_back(t1);
            if (t2 >= 0 && t2 <= 1) roots.push_back(t2);
        }
    }
    
    return roots;

}

inline float solveLinear(float a, float b){
    auto array = solveQuadratic(0, a, b);
    if (array.size() == 0){
        return -1;
    }
    return array[0];
}

inline GRect computeQuadBounds(const GPoint pts[3]) {
    float minX = std::min(pts[0].x, pts[2].x);
    float maxX = std::max(pts[0].x, pts[2].x);
    float minY = std::min(pts[0].y, pts[2].y);
    float maxY = std::max(pts[0].y, pts[2].y);

    // Quadratic bezier derivatives for x and y
    GPoint a = 2*pts[0] - 4*pts[1] + 2*pts[2];
    GPoint b = 2 * (pts[1]- pts[0]);

    // Solve for t in [0, 1] where derivatives are zero (extrema)
    float t = solveLinear(a.x, b.x);
    if (t >= 0 && t <= 1) {
        float xt = (1 - t) * (1 - t) * pts[0].x + 2 * (1 - t) * t * pts[1].x + t * t * pts[2].x;
        minX = std::min(minX, xt);
        maxX = std::max(maxX, xt);
    }
    

    t = solveLinear(a.y, b.y);
    if (t >= 0 && t <= 1) {
        float yt = (1 - t) * (1 - t) * pts[0].y + 2 * (1 - t) * t * pts[1].y + t * t * pts[2].y;
        minY = std::min(minY, yt);
        maxY = std::max(maxY, yt);
    }
    

    return GRect::LTRB(minX, minY, maxX, maxY);
}

inline GRect computeCubicBounds(const GPoint pts[4]) {
    float minX = std::min({pts[0].x, pts[3].x});
    float maxX = std::max({pts[0].x, pts[3].x});
    float minY = std::min({pts[0].y, pts[3].y});
    float maxY = std::max({pts[0].y, pts[3].y});

    // Cubic bezier derivatives for x and y
    float ax = -pts[0].x + 3 * pts[1].x - 3 * pts[2].x + pts[3].x;
    float bx = 2 * (pts[0].x - 2 * pts[1].x + pts[2].x);
    float cx = pts[1].x - pts[0].x;
    float ay = -pts[0].y + 3 * pts[1].y - 3 * pts[2].y + pts[3].y;
    float by = 2 * (pts[0].y - 2 * pts[1].y + pts[2].y);
    float cy = pts[1].y - pts[0].y;

    // Solve for t in [0, 1] where derivatives are zero (extrema)
    for (float t : solveQuadratic(ax, bx, cx)) {
        if (t >= 0 && t <= 1) {
            float xt = (1 - t) * (1 - t) * (1 - t) * pts[0].x +
                       3 * (1 - t) * (1 - t) * t * pts[1].x +
                       3 * (1 - t) * t * t * pts[2].x +
                       t * t * t * pts[3].x;
            minX = std::min(minX, xt);
            maxX = std::max(maxX, xt);
        }
    }
    for (float t : solveQuadratic(ay, by, cy)) {
        if (t >= 0 && t <= 1) {
            float yt = (1 - t) * (1 - t) * (1 - t) * pts[0].y +
                       3 * (1 - t) * (1 - t) * t * pts[1].y +
                       3 * (1 - t) * t * t * pts[2].y +
                       t * t * t * pts[3].y;
            minY = std::min(minY, yt);
            maxY = std::max(maxY, yt);
        }
    }

    return GRect::LTRB(minX, minY, maxX, maxY);
}


// Helper function to compute the union of two rectangles
inline GRect computeUnionBounds(const GRect& rect1, const GRect& rect2) {

    float left = std::min(rect1.left, rect2.left);
    float top = std::min(rect1.top, rect2.top);
    float right = std::max(rect1.right, rect2.right);
    float bottom = std::max(rect1.bottom, rect2.bottom);

    return GRect::LTRB(left, top, right, bottom);
}

// Helper function to perform linear interpolation between two points
inline GPoint lerp(const GPoint& p0, const GPoint& p1, float t) {
    return {
        p0.x * (1 - t) + p1.x * t,
        p0.y * (1 - t) + p1.y * t
    };
}

// Helper function to flatten a quadratic curve with adaptive subdivision
inline void flattenQuad(const GPoint src[3], std::vector<Edge>& edges, const GMatrix& matrix, float tolerance) {
    // Calculate midpoint for flatness test
    GPoint mid = lerp(lerp(src[0], src[1], 0.5f), lerp(src[1], src[2], 0.5f), 0.5f);

    // Check if midpoint is within tolerance of the line segment endpoints
    float dist = std::sqrt((mid.x - src[1].x) * (mid.x - src[1].x) + (mid.y - src[1].y) * (mid.y - src[1].y));
    if (dist <= tolerance) {
        // If flat, map points and add edge
        GPoint mappedPoints[2] = { src[0], src[2] };
        matrix.mapPoints(mappedPoints, mappedPoints, 2);
        addEdge(edges, mappedPoints[0], mappedPoints[1]);
    } else {
        // Otherwise, subdivide and flatten each part
        GPoint dst[5];
        GPath::ChopQuadAt(src, dst, 0.5f);
        flattenQuad(dst, edges, matrix, tolerance);       // Left half
        flattenQuad(dst + 2, edges, matrix, tolerance);   // Right half
    }
}

// Helper function to flatten a cubic curve with adaptive subdivision
inline void flattenCubic(const GPoint src[4], std::vector<Edge>& edges, const GMatrix& matrix, float tolerance) {
    // Calculate midpoints for flatness test
    GPoint mid1 = lerp(src[0], src[3], 0.5f);
    GPoint mid2 = lerp(src[1], src[2], 0.5f);
    float dist = std::sqrt((mid1.x - mid2.x) * (mid1.x - mid2.x) + (mid1.y - mid2.y) * (mid1.y - mid2.y));

    if (dist <= tolerance) {
        // If flat, map points and add edge
        GPoint mappedPoints[2] = { src[0], src[3] };
        matrix.mapPoints(mappedPoints, mappedPoints, 2);
        addEdge(edges, mappedPoints[0], mappedPoints[1]);
    } else {
        // Otherwise, subdivide and flatten each part
        GPoint dst[7];
        GPath::ChopCubicAt(src, dst, 0.5f);
        flattenCubic(dst, edges, matrix, tolerance);       // Left half
        flattenCubic(dst + 3, edges, matrix, tolerance);   // Right half
    }
}

inline float tileClamp(float value, float max) {
    return std::max(0.0f, std::min(value, max));
}

inline float tileRepeat(float value, float max) {
    return value - std::floor(value / max) * max;
}

inline float tileMirror(float value, float max) {
    float mod = tileRepeat(value, max * 2);
    return mod > max ? 2 * max - mod : mod;
}



// Helper to interpolate between two colors
inline GColor lerpColor(const GColor& c0, const GColor& c1, float t) {
    return GColor::RGBA(
        c0.r * (1 - t) + c1.r * t,
        c0.g * (1 - t) + c1.g * t,
        c0.b * (1 - t) + c1.b * t,
        c0.a * (1 - t) + c1.a * t
    ).pinToUnit();
}

/*// Tessellate a quad into triangles
inline void tessellateQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                           int level, std::vector<GPoint>& outVerts, std::vector<GColor>& outColors,
                           std::vector<GPoint>& outTexs, std::vector<int>& outIndices) {
    int n = 1 << level;  // Number of subdivisions per side
    float step = 1.0f / n;

    // Generate vertices
    for (int y = 0; y <= n; ++y) {
        for (int x = 0; x <= n; ++x) {
            float u = x * step;
            float v = y * step;

            // Interpolate positions
            GPoint pt = lerp(lerp(verts[0], verts[1], u), lerp(verts[3], verts[2], u), v);
            outVerts.push_back(pt);

            // Interpolate colors
            if (colors) {
                GColor colTop = lerpColor(colors[0], colors[1], u);
                GColor colBottom = lerpColor(colors[3], colors[2], u);
                GColor col = lerpColor(colTop, colBottom, v); // Interpolate vertically
                outColors.push_back(col);
            }


            // Interpolate texture coordinates
            if (texs) {
                GPoint tex = lerp(lerp(texs[0], texs[1], u), lerp(texs[3], texs[2], u), v);
                outTexs.push_back(tex);
            }
        }
    }

    // Generate triangle indices
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            int topLeft = y * (n + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + (n + 1);
            int bottomRight = bottomLeft + 1;

            outIndices.push_back(topLeft);
            outIndices.push_back(topRight);
            outIndices.push_back(bottomRight);

            outIndices.push_back(topLeft);
            outIndices.push_back(bottomRight);
            outIndices.push_back(bottomLeft);
        }
    }
}*/

// Tessellate a quad into triangles
inline void tessellateQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                           int level, std::vector<GPoint>& outVerts, std::vector<GColor>& outColors,
                           std::vector<GPoint>& outTexs, std::vector<int>& outIndices) {
    int n = level+1;  // Number of subdivisions per side
    // int n = 1 << std::min(4, level);  // Number of subdivisions per side, hard coded
    // Threshold to stop adding degenerate triangles
    constexpr float epsilon = 1e-5f;
    float step = std::max(1.0f / n, epsilon); // Look into step and epsilon usage

    

    // Generate vertices
    for (int y = 0; y <= n; ++y) {
        for (int x = 0; x <= n; ++x) {
            float u = x * step;
            float v = y * step;

            // Interpolate positions
            GPoint pt = lerp(lerp(verts[0], verts[1], u), lerp(verts[3], verts[2], u), v);
            outVerts.push_back(pt);

            // Interpolate colors
            if (colors) {
                GColor colTop = lerpColor(colors[0], colors[1], u);
                GColor colBottom = lerpColor(colors[3], colors[2], u);
                GColor col = lerpColor(colTop, colBottom, v);  // Interpolate vertically
                outColors.push_back(col);
            }

            // Interpolate texture coordinates
            if (texs) {
                GPoint tex = lerp(lerp(texs[0], texs[1], u), lerp(texs[3], texs[2], u), v);
                outTexs.push_back(tex);
            }
        }
    }

    // Generate triangle indices
    for (int y = 0; y < n; ++y) {
        for (int x = 0; x < n; ++x) {
            int topLeft = y * (n + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = topLeft + (n + 1);
            int bottomRight = bottomLeft + 1;

            // Compute area of each triangle and avoid degenerate triangles
            float area1 = fabsf((outVerts[topLeft].x * (outVerts[topRight].y - outVerts[bottomRight].y) +
                                 outVerts[topRight].x * (outVerts[bottomRight].y - outVerts[topLeft].y) +
                                 outVerts[bottomRight].x * (outVerts[topLeft].y - outVerts[topRight].y)) * 0.5f);

            float area2 = fabsf((outVerts[topLeft].x * (outVerts[bottomRight].y - outVerts[bottomLeft].y) +
                                 outVerts[bottomRight].x * (outVerts[bottomLeft].y - outVerts[topLeft].y) +
                                 outVerts[bottomLeft].x * (outVerts[topLeft].y - outVerts[bottomRight].y)) * 0.5f);

            if (area1 > epsilon) {
                outIndices.push_back(topLeft);
                outIndices.push_back(topRight);
                outIndices.push_back(bottomRight);
            }

            if (area2 > epsilon) {
                outIndices.push_back(topLeft);
                outIndices.push_back(bottomRight);
                outIndices.push_back(bottomLeft);
            }
        }
    }
}


// Calculate the texture transformation matrix
inline GMatrix calculateTextureTransform(const GPoint& t0, const GPoint& t1, const GPoint& t2,
                                         const GPoint& p0, const GPoint& p1, const GPoint& p2) {
    GMatrix texToUnit = GMatrix(t1.x - t0.x, t2.x - t0.x, t0.x,
                                 t1.y - t0.y, t2.y - t0.y, t0.y);
    // Check for invertibility of texToUnit
    auto invTexToUnit = texToUnit.invert();
    if (!invTexToUnit) {
        printf("Error: texToUnit matrix is not invertible.\n");
        return GMatrix();
    }
    GMatrix triToCanvas = GMatrix(p1.x - p0.x, p2.x - p0.x, p0.x,
                                  p1.y - p0.y, p2.y - p0.y, p0.y);
    return GMatrix::Concat(triToCanvas, texToUnit.invert().value());
}

inline GMatrix compute_basis(const GPoint& p0, const GPoint& p1, const GPoint& p2) {
    return GMatrix(p1.x - p0.x, p2.x - p0.x, p0.x,
                   p1.y - p0.y, p2.y - p0.y, p0.y);
}

inline float computeTriangleArea(const GPoint& p0, const GPoint& p1, const GPoint& p2) {
    return 0.5f * (p0.x * (p1.y - p2.y) + p1.x * (p2.y - p0.y) + p2.x * (p0.y - p1.y));
}

#endif