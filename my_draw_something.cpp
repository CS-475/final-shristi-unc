// /*
//  *  Copyright 2024 Shristi
//  */

// #include "include/GCanvas.h"
// #include "include/GRect.h"
// #include "include/GColor.h"
// #include "include/GPoint.h"
// #include "include/GPaint.h"
// #include "include/GMath.h"
// #include "include/GPathBuilder.h"
// #include "include/GShader.h"
// #include <string>

// // Function to draw something on the canvas
// std::string GDrawSomething(GCanvas* canvas, GISize dim) {
//     // Clear the canvas with a base color
//     GColor baseColor = GColor::RGBA(0.8f, 0.8f, 1.0f, 1.0f);  // Light purple
//     canvas->clear(baseColor);

//     /* // Draw a large red rectangle
//     GColor red = GColor::RGBA(1.0f, 0.0f, 0.0f, 1.0f);  // Solid red
//     GRect largeRect = GRect::LTRB(90, 90, dim.width - 90, dim.height - 90);
//     canvas->fillRect(largeRect, red);

//     // Draw a smaller green rectangle inside the large one
//     GColor green = GColor::RGBA(0.0f, 1.0f, 0.0f, 1.0f);  // Solid green
//     GRect smallRect = GRect::LTRB(100, 100, dim.width - 100, dim.height - 100);
//     canvas->fillRect(smallRect, green);

//     */

//     /* // Draw a convex polygon (e.g., a triangle)
//     GPaint paint;
//     paint.setColor(GColor::RGBA(1.0f, 0.0f, 0.0f, 1.0f));  // Solid red

//     GPoint triangle[3] = {
//         {100, 100},
//         {dim.width - 100, 100},
//         {dim.width / 2, dim.height - 100},
//     };

//     canvas->drawConvexPolygon(triangle, 3, paint);

//     // Return a title for the artwork
//     return "Convex Polygon Art (Triangle)";
//     */

//     /* // Translated Red Rectangle
//     // Set up a red paint color
//     GPaint paint;
//     paint.setColor(GColor::RGBA(1.0f, 0.0f, 0.0f, 1.0f));  // Solid red

//     // Define the rectangle
//     GRect rect = GRect::XYWH(50, 50, 100, 100);  // 100x100 rectangle at (50, 50)

//     // Draw the first rectangle without any transformations
//     canvas->drawRect(rect, paint);

//     // Apply a translation and draw the second rectangle
//     canvas->save();
//     canvas->translate(50, 50);
//     canvas->drawRect(rect, paint);  // Draw the translated rectangle
//     canvas->restore();

//     return "Translated Red Rectangle";
//     */

//     /*// Draw a gradient-filled rectangle
//     GPoint p0 = {50, 50};
//     GPoint p1 = {dim.width - 50, dim.height - 50};
//     GColor colors[] = {GColor::RGB(1, 0, 0), GColor::RGB(0, 0, 1)};  // Red to blue

//     std::shared_ptr<GShader> shader = GCreateLinearGradient(p0, p1, colors, 2);
//     GPaint paint;
//     paint.setShader(shader);

//     GPathBuilder builder;
//     builder.addRect(GRect::XYWH(50, 50, 100, 100), GPathDirection::kCW);
//     std::shared_ptr<GPath> path = builder.detach();
    
//     canvas->drawPath(*path, paint);

//     return "Gradient-Filled Rectangle";*/
    
//     // Paint setup for drawing paths
//     GPaint paint;
//     paint.setColor(GColor::RGBA(0.0f, 0.5f, 0.5f, 1.0f));  // Teal

//     // Test 1: Draw a simple quadratic curve
//     GPathBuilder quadBuilder;
//     quadBuilder.moveTo(50, 50);  // Set the starting point of the path
//     quadBuilder.quadTo(150, 50, 100, 100);  // Add a quadratic curve
//     auto quadPath = quadBuilder.detach();

//     // Draw the quadratic path
//     canvas->drawPath(*quadPath, paint);

//     // Test bounds for the quadratic curve
//     GRect quadBounds = quadPath->bounds();
//     paint.setColor(GColor::RGBA(0.8f, 0.0f, 0.0f, 0.5f));  // Semi-transparent red for bounds
//     canvas->fillRect(quadBounds, paint.getColor());

//     // Test 2: Draw a cubic curve
//     GPathBuilder cubicBuilder;
//     cubicBuilder.moveTo(50, 150);
//     cubicBuilder.cubicTo(100, 50, 200, 50, 150, 150);

//     auto cubicPath = cubicBuilder.detach();

//     // Draw the cubic path
//     paint.setColor(GColor::RGBA(0.0f, 0.0f, 1.0f, 1.0f));  // Solid blue
//     canvas->drawPath(*cubicPath, paint);

//     // Test bounds for the cubic curve
//     GRect cubicBounds = cubicPath->bounds();
//     paint.setColor(GColor::RGBA(0.0f, 0.8f, 0.0f, 0.5f));  // Semi-transparent green for bounds
//     canvas->fillRect(cubicBounds, paint.getColor());

//     // Test 3: Draw a gradient-filled rectangle for comparison
//     GPoint p0 = {50, 50};
//     GPoint p1 = {dim.width - 50, dim.height - 50};
//     GColor colors[] = {GColor::RGB(1, 0, 0), GColor::RGB(0, 0, 1)};  // Red to blue gradient
//     std::shared_ptr<GShader> shader = GCreateLinearGradient(p0, p1, colors, 2);
//     paint.setShader(shader);

//     GPathBuilder rectBuilder;
//     rectBuilder.addRect(GRect::XYWH(200, 200, 100, 100), GPathDirection::kCW);
//     std::shared_ptr<GPath> rectPath = rectBuilder.detach();

//     // Draw the gradient-filled rectangle
//     canvas->drawPath(*rectPath, paint);

//     return "PA5 Tests: Quad, Cubic, and Gradient Rectangle";
// }

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GPoint.h"
#include "include/GPaint.h"
#include <vector>
#include <string>

// Function to draw something on the canvas
std::string GDrawSomething(GCanvas* canvas, GISize dim) {
    // Clear the canvas with a base color
    GColor baseColor = GColor::RGBA(0.9f, 0.9f, 0.9f, 1.0f);  // Light gray
    canvas->clear(baseColor);

    // Define the quad vertices
    GPoint verts[4] = {
        {50, 50},  // Top-left
        {150, 50},  // Top-right
        {150, 150},  // Bottom-right
        {50, 150}   // Bottom-left
    };

    // Define the colors for each vertex
    GColor colors[4] = {
        GColor::RGBA(1.0f, 0.0f, 0.0f, 1.0f),  // Red
        GColor::RGBA(0.0f, 1.0f, 0.0f, 1.0f),  // Green
        GColor::RGBA(0.0f, 0.0f, 1.0f, 1.0f),  // Blue
        GColor::RGBA(1.0f, 1.0f, 0.0f, 1.0f)   // Yellow
    };

    // Define texture coordinates (optional, pass nullptr if no texture is used)
    GPoint texs[4] = {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    };

    // Set up paint
    GPaint paint;

    // Draw the quad with tessellation level 1
    int level = 4;  // Tessellation level
    canvas->drawQuad(verts, colors, texs, level, paint);

    return "Simple Quad Test";
}
