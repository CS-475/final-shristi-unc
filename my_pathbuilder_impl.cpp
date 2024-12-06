#include "include/GPathBuilder.h"
#include "include/GPath.h"

void GPathBuilder::addRect(const GRect& r, GPathDirection dir) {
    if (dir == GPathDirection::kCW) {
        // Clockwise
        moveTo(r.left, r.top);
        lineTo(r.right, r.top);
        lineTo(r.right, r.bottom);
        lineTo(r.left, r.bottom);
    } else {
        // Counter-clockwise
        moveTo(r.left, r.top);
        lineTo(r.left, r.bottom);
        lineTo(r.right, r.bottom);
        lineTo(r.right, r.top);
    }
    
    // // Explicitly close the rectangle contour by connecting back to the first point
    // lineTo(r.left, r.top);
}


void GPathBuilder::addPolygon(const GPoint pts[], int count) {
    moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        lineTo(pts[i]);
    }
    // Close the polygon by connecting back to the starting point
    // lineTo(pts[0]);
}

void GPathBuilder::addCircle(GPoint center, float radius, GPathDirection dir) {
    // Control points for one quadrant of a unit circle
    const float kSqrt2Over2 = sqrt(2) / 2;  // sqrt(2)/2
    const float kTanPiOver8 = tan(M_PI / 8);  // tan(pi/8)

    // Control points for a circle approximated with quadratics
    GPoint quadPts[3] = {
        {1, 0},                 // Start point
        {1, kTanPiOver8},       // Control point
        {kSqrt2Over2, kSqrt2Over2} // End point of first segment
    };

    GMatrix mx = GMatrix::Translate(center.x, center.y) * GMatrix::Scale(radius, radius);

    moveTo(mx * quadPts[0]);
    // Helper function to add a quadratic segment
    auto addQuad = [&](GPoint control, GPoint end) {
        quadTo(control, end);
    };

    // Quadrants are defined clockwise from (1, 0)
    if (dir == GPathDirection::kCW) {
        addQuad(mx * quadPts[1], mx * quadPts[2]);
        addQuad(mx * GPoint{kTanPiOver8, 1}, mx * GPoint{0, 1});
        addQuad(mx * GPoint{-kTanPiOver8, 1}, mx * GPoint{-kSqrt2Over2, kSqrt2Over2});
        addQuad(mx * GPoint{-1, kTanPiOver8}, mx * GPoint{-1, 0});
        addQuad(mx * GPoint{-1, -kTanPiOver8}, mx * GPoint{-kSqrt2Over2, -kSqrt2Over2});
        addQuad(mx * GPoint{-kTanPiOver8, -1}, mx * GPoint{0, -1});
        addQuad(mx * GPoint{kTanPiOver8, -1}, mx * GPoint{kSqrt2Over2, -kSqrt2Over2});
        addQuad(mx * GPoint{1, -kTanPiOver8}, mx * GPoint{1, 0});
    } else {
        // Counter-clockwise direction
        addQuad(mx * GPoint{1, -kTanPiOver8}, mx * GPoint{kSqrt2Over2, -kSqrt2Over2});
        addQuad(mx * GPoint{kTanPiOver8, -1}, mx * GPoint{0, -1});
        addQuad(mx * GPoint{-kTanPiOver8, -1}, mx * GPoint{-kSqrt2Over2, -kSqrt2Over2});
        addQuad(mx * GPoint{-1, -kTanPiOver8}, mx * GPoint{-1, 0});
        addQuad(mx * GPoint{-1, kTanPiOver8}, mx * GPoint{-kSqrt2Over2, kSqrt2Over2});
        addQuad(mx * GPoint{-kTanPiOver8, 1}, mx * GPoint{0, 1});
        addQuad(mx * GPoint{kTanPiOver8, 1}, mx * GPoint{kSqrt2Over2, kSqrt2Over2});
        addQuad(mx * quadPts[1], mx * quadPts[0]);
    }
}

