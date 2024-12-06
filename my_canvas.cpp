/*
 *  Copyright 2024 Shristi
 */

#include "starter_canvas.h"
#include "my_utils.h"
#include "include/GMath.h"
#include "include/GBitmap.h"
#include "include/GBlendMode.h"
#include "include/GPaint.h"
#include "include/GMatrix.h"
#include "include/GShader.h"
#include "include/GCanvas.h"
#include "include/GPath.h"
#include "ProxyShader.h"
#include "TriColorShader.h"
#include "CompositeShader.h"
#include <stack>
#include <iostream>
#include <memory>
#include <algorithm>

void MyCanvas::save() {
    fMatrixStack.push(fMatrixStack.top());  // Save current CTM
}

void MyCanvas::restore() {
    if (fMatrixStack.size() > 1) {
        fMatrixStack.pop();  // Restore previous CTM
    }
}

void MyCanvas::concat(const GMatrix& matrix) {
    fMatrixStack.top() = GMatrix::Concat(fMatrixStack.top(), matrix);
}

void MyCanvas::clear(const GColor& color) {
    // Convert GColor to GPixel (with our helper func)
    GPixel pixel = ColorToPixel(color);

    // Optimized: Fill entire bitmap with the color
    GPixel *row_addr = nullptr;
    int width = fDevice.width();
    int height = fDevice.height();
    for (int y = 0; y < height; y++) {
        row_addr = fDevice.getAddr(0, y);
        for (int x = 0; x < width; x++) {
            row_addr[x] = pixel;
        }
    }
}

void MyCanvas::fillRectX(const GRect& rect, const GColor& color) {
    // Convert GColor to GPixel (with our helper func)
    GPixel srcPixel = ColorToPixel(color);

    int srcA = GPixel_GetA(srcPixel);
    int srcR = GPixel_GetR(srcPixel);
    int srcG = GPixel_GetG(srcPixel);
    int srcB = GPixel_GetB(srcPixel);

    // If source alpha is 0, nothing to draw (fully transparent)
    if (srcA == 0){
        return;
    }

    // Use floating-point values for precision
    float left = rect.left;
    float top = rect.top;
    float right = rect.right;
    float bottom = rect.bottom;
    
    int width = fDevice.width();
    int height = fDevice.height();

    // Check if the rectangle is completely outside the canvas bounds
    if (right <= 0 || bottom <= 0 || left >= width || top >= height) {
        return; // The rectangle is fully clipped out
    }

    // Loop through each pixel in the specified rectangle bounds
    for (int y = 0; y < height; y++) {
        float pixelCenterY = y + 0.5f;  // Center of the pixel row
        if (pixelCenterY > top && pixelCenterY <= bottom) {
            GPixel* row_addr = fDevice.getAddr(0, y);
            for (int x = 0; x < width; x++) {
                float pixelCenterX = x + 0.5f;  // Center of the pixel column

                // Check if the pixel center is inside the rectangle
                if (pixelCenterX > left && pixelCenterX <= right) {
                    GPixel* dstAddr = &row_addr[x];
                    GPixel dstPixel = *dstAddr;

                    // Optimization: If the source is fully opaque, replace the destination directly
                    if (srcA == 255) {
                        *dstAddr = srcPixel;  // Direct copy if source is fully opaque
                    } else {
                        // Extract destination components
                        int dstA = GPixel_GetA(dstPixel);
                        int dstR = GPixel_GetR(dstPixel);
                        int dstG = GPixel_GetG(dstPixel);
                        int dstB = GPixel_GetB(dstPixel);

                        // Apply blending formulas using integer math for color components
                        int outA = srcA + (dstA * (255 - srcA) / 255);
                        int outR = srcR + (dstR * (255 - srcA) / 255);
                        int outG = srcG + (dstG * (255 - srcA) / 255);
                        int outB = srcB + (dstB * (255 - srcA) / 255);

                        // Pack the result back into a pixel
                        *dstAddr = GPixel_PackARGB(outA, outR, outG, outB);
                    }
                }
            }
        }
    }
}

void MyCanvas::drawRect(const GRect& rect, const GPaint& paint) {
    // GBlendMode blendMode = paint.getBlendMode();
    // int width = fDevice.width();
    // int height = fDevice.height();

    // Transform the rectangle by the current transformation matrix (CTM)
    GPoint pts[4] = {
        {rect.left, rect.top},
        {rect.right, rect.top},
        {rect.right, rect.bottom},
        {rect.left, rect.bottom},
    };

    // Call drawConvexPolygon using these points
    this->drawConvexPolygon(pts, 4, paint);

    // fMatrixStack.top().mapPoints(pts, pts, 4);

    // // Compute the bounds after transformation
    // GRect bounds = computeBounds(pts, 4);

    // // Ensure the rectangle is within the bounds of the canvas
    // int left = std::max(0, static_cast<int>(bounds.left));
    // int right = std::min(width, static_cast<int>(bounds.right));
    // int top = std::max(0, static_cast<int>(bounds.top));
    // int bottom = std::min(height, static_cast<int>(bounds.bottom));

    // // Set up the shader if available
    // GShader* shader = paint.peekShader();
    // if (shader && shader->setContext(fMatrixStack.top())) {
    //     for (int y = top; y < bottom; ++y) {
    //         GPixel* row_addr = fDevice.getAddr(0, y);
    //         GPixel row[right-left];
    //         shader->shadeRow(left, y, right - left, row);  // Get the shader pixels

    //         for (int x = left; x < right; ++x) {
    //             GPixel* dstAddr = &row_addr[x];
    //             *dstAddr = applyBlendMode(row[x - left], *dstAddr, blendMode);
    //         }
    //     }
    // } else {
    //     // Use the paint's color if no shader is present
    //     GPixel srcPixel = ColorToPixel(paint.getColor());
    //     for (int y = 0; y < height; y++) {
    //         float pixelCenterY = y + 0.5f;
    //         if (pixelCenterY > rect.top && pixelCenterY <= rect.bottom) {
    //             GPixel* row_addr = fDevice.getAddr(0, y);
    //             for (int x = 0; x < width; x++) {
    //                 float pixelCenterX = x + 0.5f;

    //                 if (pixelCenterX > rect.left && pixelCenterX <= rect.right) {
    //                     GPixel* dstAddr = &row_addr[x];
    //                     *dstAddr = applyBlendMode(srcPixel, *dstAddr, blendMode);
    //                 }
    //             }
    //         }
    //     }
    // }
}


void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) {
    GBlendMode blendMode = paint.getBlendMode();

    if (paint.peekShader() == NULL) {
        blendMode = optimize(blendMode, paint.getAlpha());
    }

    // Early exit if blend mode results in no changes
    if (blendMode == GBlendMode::kDst) {
        return;  // No need to draw
    }

    int width = fDevice.width();
    int height = fDevice.height();

    // Transform the polygon points by the current transformation matrix (CTM)
    GPoint transformedPoints[count];
    fMatrixStack.top().mapPoints(transformedPoints, points, count);

    // Compute the bounds after transformation
    GRect bounds = computeBounds(transformedPoints, count);

    // Ensure the polygon is within the bounds of the canvas
    int top = std::max(0, static_cast<int>(bounds.top));
    int bottom = std::min(height, static_cast<int>(bounds.bottom));

    // Sort edges for scanline rasterization
    std::vector<Edge> edges;
    for (int i = 0; i < count; ++i) {
        int j = (i + 1) % count;    // Wrap around to create a closed polygon
        addEdge(edges, transformedPoints[i], transformedPoints[j]);
    }
    std::sort(edges.begin(), edges.end(), compareEdges);

    // Rasterize the polygon using blit
    for (int y = top; y < bottom; ++y) {
        std::vector<int> intersections;

        // Calculate intersections of edges with the current scanline
        for (const auto& edge : edges) {
            if (edge.top <= y && edge.bottom > y) {
                float x = edge.computeX(y);
                intersections.push_back(static_cast<int>(std::round(x)));
            }
        }

        // Sort intersections to determine the active spans to fill
        if (intersections.size() >= 2) {
            std::sort(intersections.begin(), intersections.end());
            int left = std::max(0, intersections.front());
            int right = std::min(width, intersections.back());

            // Use the blit function for shading between the intersections
            blit(left, y, right - left, paint, fDevice, fMatrixStack.top());  // Shade the span using blit
        }
    }
}

// Handle winding-based non-convex polygons
void MyCanvas::drawPath(const GPath& path, const GPaint& paint) {
    std::vector<Edge> edges;
    GPath::Edger edger(path);
    GPoint points[4];
    const float tolerance = 0.25f;  // Tolerance of 1/4 pixel

    /* // Check if the paint uses a StrokeShader
    auto shader = dynamic_cast<StrokeShader*>(paint.peekShader());
    if (shader) {
        // Collect all points and flatten curves from the path
        std::vector<GPoint> originalPoints;

        while (auto verb = edger.next(points)) {
            switch (verb.value()) {
                case GPathVerb::kLine: {
                    // Map points to canvas space and add to originalPoints
                    fMatrixStack.top().mapPoints(points, points, 2);
                    originalPoints.push_back(points[0]);
                    originalPoints.push_back(points[1]);
                    break;
                }
                case GPathVerb::kQuad: {
                    // Flatten the quadratic curve into line segments
                    std::vector<Edge> tempEdges;
                    flattenQuad(points, tempEdges, fMatrixStack.top(), tolerance);

                    for (const auto& edge : tempEdges) {
                        originalPoints.push_back({edge.computeX(edge.top), (float)edge.top});
                        originalPoints.push_back({edge.computeX(edge.bottom), (float)edge.bottom});
                    }
                    break;
                }
                case GPathVerb::kCubic: {
                    // Flatten the cubic curve into line segments
                    std::vector<Edge> tempEdges;
                    flattenCubic(points, tempEdges, fMatrixStack.top(), tolerance);

                    for (const auto& edge : tempEdges) {
                        originalPoints.push_back({edge.computeX(edge.top), (float)edge.top});
                        originalPoints.push_back({edge.computeX(edge.bottom), (float)edge.bottom});
                    }
                    break;
                }
                default:
                    break;
            }
        }

        // Generate the stroked polygon using the shader
        std::vector<GPoint> strokedPoints;
        shader->generateStrokedPolygon(originalPoints.data(), originalPoints.size(), strokedPoints);

        // Draw the stroked polygon as a convex polygon
        this->drawConvexPolygon(strokedPoints.data(), strokedPoints.size(), paint);
        return;
    }*/

    // Extract all edges from the path using the edger
    while (auto verb = edger.next(points)) {
        switch (verb.value()) {
            case GPathVerb::kLine:
                // Map points to canvas space and add line edge
                fMatrixStack.top().mapPoints(points, points, 2);
                addEdge(edges, points[0], points[1]);
                break;

            case GPathVerb::kQuad:
                // Recursively flatten the quadratic curve into line segments
                flattenQuad(points, edges, fMatrixStack.top(), tolerance);
                break;

            case GPathVerb::kCubic:
                // Recursively flatten the cubic curve into line segments
                flattenCubic(points, edges, fMatrixStack.top(), tolerance);
                break;

            default:
                break;
        }
    }

    // Sort edges by their top Y values and then by their X values
    std::sort(edges.begin(), edges.end(), compareEdges);

    if (edges.empty()) return; // Avoid out-of-bounds access

    int yMin = edges[0].top;
    int yMax = edges[0].bottom;

    // Find the max Y value
    for (const auto& edge : edges) {
        yMax = std::max(yMax, edge.bottom);
    }

    // Active edge list for edges at each scanline
    std::vector<Edge> activeEdges;

    // Scanline-based rendering: Process each Y value from yMin to yMax
    for (int y = yMin; y < yMax; ++y) {
        // Remove edges that are no longer valid for the next scanline
        activeEdges.erase(std::remove_if(activeEdges.begin(), activeEdges.end(),
            [y](const Edge& edge) { return !edge.isValid(y); }), activeEdges.end());

        // Add new edges that are active at this scanline
        for (const auto& edge : edges) {
            if (edge.top == y) {
                activeEdges.push_back(edge);
            }
        }

        // Sort active edges by their current X values
        std::sort(activeEdges.begin(), activeEdges.end(), [&y](const Edge& a, const Edge& b) {
            return a.computeX(y) < b.computeX(y);
        });

        int winding = 0;
        int leftX = 0;
 
        // Process the active edges for this scanline
        for (size_t i = 0; i < activeEdges.size(); ++i) {
            const Edge& edge = activeEdges[i];
            int x = GRoundToInt(edge.computeX(y));

            if (winding == 0) {
                leftX = x;  // Start a new span
            }

            winding += edge.winding;  // Update the winding value

            if (winding == 0) {
                // End of a filled span (when winding becomes zero)
                int rightX = x;
                if (rightX > leftX) {  // Ensure we're drawing in the correct order
                    blit(leftX, y, rightX - leftX, paint, fDevice, fMatrixStack.top());
                }
            }
        }
    }

}

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) {
    GShader* shader = paint.peekShader();
    bool hasShader = shader != nullptr;
    bool hasColors = colors != nullptr;
    bool hasTexs = texs != nullptr && hasShader;

    // If there are neither colors nor textures, nothing can be drawn
    if (!hasColors && !hasTexs) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        // Extract triangle vertices
        GPoint p0 = verts[indices[i * 3 + 0]];
        GPoint p1 = verts[indices[i * 3 + 1]];
        GPoint p2 = verts[indices[i * 3 + 2]];

        // Extract associated colors
        GColor c0 = hasColors ? colors[indices[i * 3 + 0]] : GColor::RGBA(0, 0, 0, 0);
        GColor c1 = hasColors ? colors[indices[i * 3 + 1]] : GColor::RGBA(0, 0, 0, 0);
        GColor c2 = hasColors ? colors[indices[i * 3 + 2]] : GColor::RGBA(0, 0, 0, 0);

        // Prepare texture coordinates
        GPoint tex[3] = {
            hasTexs ? texs[indices[i * 3 + 0]] : GPoint{0, 0},
            hasTexs ? texs[indices[i * 3 + 1]] : GPoint{0, 0},
            hasTexs ? texs[indices[i * 3 + 2]] : GPoint{0, 0}
        };

        // Determine the paint to use
        GPaint finalPaint = paint;

        if (hasColors && hasTexs) {
            // Compute the texture-to-canvas transformation
            GMatrix texToCanvas = calculateTextureTransform(tex[0], tex[1], tex[2], p0, p1, p2);
            auto proxyShader = std::make_shared<ProxyShader>(shader, texToCanvas);

            // Create the TriColorShader
            GPoint pts[] = {p0, p1, p2};
            GColor cols[] = {c0, c1, c2};
            auto triColorShader = std::make_shared<TriColorShader>(pts, cols);

            // Create a composite shader
            auto compositeShader = std::make_shared<CompositeShader>(triColorShader, proxyShader);
            finalPaint = GPaint(compositeShader);
        } else if (hasTexs) {
            // Only texture shader
            GMatrix texToCanvas = calculateTextureTransform(tex[0], tex[1], tex[2], p0, p1, p2);
            auto proxyShader = std::make_shared<ProxyShader>(shader, texToCanvas);
            finalPaint = GPaint(proxyShader);
        } else if (hasColors) {
            // Only color shader
            GPoint pts[] = {p0, p1, p2};
            GColor cols[] = {c0, c1, c2};
            auto triColorShader = std::make_shared<TriColorShader>(pts, cols);
            finalPaint = GPaint(triColorShader);
        }

        // Draw the triangle
        GPoint pts[] = {p0, p1, p2};
        drawConvexPolygon(pts, 3, finalPaint);
    }
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) {
    std::vector<GPoint> quadVerts;
    std::vector<GColor> quadColors;
    std::vector<GPoint> quadTexs;
    std::vector<int> indices;

    // Tessellate the quad
    tessellateQuad(verts, colors, texs, level, quadVerts, quadColors, quadTexs, indices);
    // Pass the tessellated quads as triangles to drawMesh
    drawMesh(quadVerts.data(),
             colors ? quadColors.data() : nullptr,
             texs ? quadTexs.data() : nullptr,
             indices.size() / 3,
             indices.data(),
             paint);
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

extern std::string GDrawSomething(GCanvas* canvas, GISize dim);