#include "include/GPath.h"
#include "include/GPathBuilder.h"
#include "my_utils.h"

// Compute tight bounds of the path
GRect GPath::bounds() const {
    if (fPts.empty()) {
        return GRect::LTRB(0, 0, 0, 0);
    }

    GRect result = GRect::LTRB(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
    GPath::Edger iter(*this);
    GPoint pts[4];

    // Loop through path segments
    while (auto verbOpt = iter.next(pts)) {
        GPathVerb verb = *verbOpt;  // Extract value from std::optional
        GRect segBounds;

        if (verb == kLine) {
            // Simple min/max of the line segment's endpoints
            float left = pts[0].x, top = pts[0].y, right = pts[0].x, bottom = pts[0].y;
            
            left = std::min(left, pts[1].x);
            top = std::min(top, pts[1].y);
            right = std::max(right, pts[1].x);
            bottom = std::max(bottom, pts[1].y);
            
            segBounds = GRect::LTRB(left, top, right, bottom);
        } else if (verb == kQuad) {
            // Compute tight bounds of the quadratic bezier
            segBounds = computeQuadBounds(pts);
        } else if (verb == kCubic) {
            // Compute tight bounds of the cubic bezier
            segBounds = computeCubicBounds(pts);
        }

        result = computeUnionBounds(result, segBounds);
    }

    return result;
}

// segBounds = computeBounds(pts, 2);

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    // Calculate midpoints for the quadratic curve
    GPoint ab = lerp(src[0], src[1], t);  // Between p0 and p1
    GPoint bc = lerp(src[1], src[2], t);  // Between p1 and p2
    GPoint abc = lerp(ab, bc, t);         // Between ab and bc (final midpoint)

    // Assign new control points for the two resulting curves
    dst[0] = src[0];   // Start of first curve
    dst[1] = ab;       // Midpoint between p0 and p1
    dst[2] = abc;      // Shared midpoint
    dst[3] = bc;       // Midpoint between p1 and p2
    dst[4] = src[2];   // End of second curve
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
    // Calculate midpoints for the cubic curve
    GPoint ab = lerp(src[0], src[1], t);  // Between p0 and p1
    GPoint bc = lerp(src[1], src[2], t);  // Between p1 and p2
    GPoint cd = lerp(src[2], src[3], t);  // Between p2 and p3

    GPoint abc = lerp(ab, bc, t);         // Between ab and bc
    GPoint bcd = lerp(bc, cd, t);         // Between bc and cd

    GPoint abcd = lerp(abc, bcd, t);      // Final midpoint

    // Assign new control points for the two resulting curves
    dst[0] = src[0];   // Start of first curve
    dst[1] = ab;       // Midpoint between p0 and p1
    dst[2] = abc;      // Midpoint between ab and bc
    dst[3] = abcd;     // Shared midpoint
    dst[4] = bcd;      // Midpoint between bc and cd
    dst[5] = cd;       // Midpoint between p2 and p3
    dst[6] = src[3];   // End of second curve
}
