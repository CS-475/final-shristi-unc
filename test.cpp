#include "include/GFinal.h"
#include "include/GPathBuilder.h"
#include "include/GPath.h"
#include "include/GMath.h"
#include "my_utils.h"
#include <memory>
#include <cmath>
#include <vector>

/*class MyFinal : public GFinal {
public:
    // Override the strokePolygon method
    std::shared_ptr<GPath> strokePolygon2(const GPoint points[], int count, float width, bool isClosed) override {
        if (count < 2) {
            return nullptr;
        }

        float halfWidth = width / 2;
        GPathBuilder builder;

        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];

            if (!isClosed && i == count - 1) {
                break;
            }

            // Calculate the normal vector
            GPoint normal = {p1.y - p0.y, p0.x - p1.x};
            float length = std::sqrt(normal.x * normal.x + normal.y * normal.y);
            if (length == 0) continue; // Avoid division by zero
            normal.x /= length;
            normal.y /= length;

            // Outset and inset points
            GPoint outsetP0 = {p0.x + halfWidth * normal.x, p0.y + halfWidth * normal.y};
            GPoint outsetP1 = {p1.x + halfWidth * normal.x, p1.y + halfWidth * normal.y};
            GPoint insetP0 = {p0.x - halfWidth * normal.x, p0.y - halfWidth * normal.y};
            GPoint insetP1 = {p1.x - halfWidth * normal.x, p1.y - halfWidth * normal.y};

            // Add outset and inset edges to the path
            builder.moveTo(outsetP0);
            builder.lineTo(outsetP1);

            if (isClosed || i < count - 1) {
                builder.lineTo(insetP1);
                builder.lineTo(insetP0);
                builder.lineTo(outsetP0); // Explicitly move back to outsetP0 to close the loop
            }

            // Add round caps between segments for closed or open polygons
            if (isClosed || i < count - 1) {
                GPoint center = p1;
                builder.addCircle(center, halfWidth, GPathDirection::kCW);
            }

            // Add round caps for the last point if open
            if (!isClosed && i == count - 1) {
                builder.addCircle(p1, halfWidth, GPathDirection::kCW);
            }
        }

        return std::make_shared<GPath>(builder.detach());
    }

    // Override other methods if necessary (e.g., createVoronoiShader, createSweepGradient)
};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::make_unique<MyFinal>();
}
*/

/*
#include "include/GFinal.h"
// #include "my_utils.h"
// #include "starter_canvas.h"
#include "include/GPathBuilder.h"
// #include "include/GPath.h"

#include <memory>
#include <cmath>
#include <vector>

class MyFinal : public GFinal {
public:
    // Override the strokePolygon method
    std::shared_ptr<GPath> strokePolygon(const GPoint points[], int count, float width, bool isClosed) {
        if (count < 2 || width <= 0) {
            return nullptr; // Invalid input
        }

        // Prepare path for the stroked polygon using a builder
        GPathBuilder builder;

        // Radius is half the stroke width
        float radius = width / 2.0f;
        const int arcSegments = 16; // Increase the number of segments for smoother arcs


        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count]; // Wrap around for closed polygons

            // Compute the direction vector and perpendicular offset
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            float length = std::sqrt(dx * dx + dy * dy);
            
            if (length == 0) continue; // Skip zero-length segments

            dx /= length;
            dy /= length;

            // Perpendicular vector
            float px = -dy * radius;
            float py = dx * radius;

            // Compute corners of the stroke rectangle
            GPoint corners[4] = {
                {p0.x + px, p0.y + py},
                {p1.x + px, p1.y + py},
                {p1.x - px, p1.y - py},
                {p0.x - px, p0.y - py}
            };

            // Add the rectangle to the path
            builder.moveTo(corners[0]);
            builder.lineTo(corners[1]);
            builder.lineTo(corners[2]);
            builder.lineTo(corners[3]);
            builder.lineTo(corners[0]); // Explicitly close the rectangle

            // Add round join at p1
            float angleStep = M_PI / arcSegments;
            for (int j = 0; j <= arcSegments; ++j) {
                float theta = j * angleStep;
                float x = p1.x + radius * (std::cos(theta) * dx - std::sin(theta) * dy);
                float y = p1.y + radius * (std::sin(theta) * dx + std::cos(theta) * dy);
                builder.lineTo(x, y);
            }
        }

        if (isClosed) {
            // Add a cap for the final segment if the polygon is closed
            GPoint firstPoint = points[0];
            GPoint lastPoint = points[count - 1];

            float dx = firstPoint.x - lastPoint.x;
            float dy = firstPoint.y - lastPoint.y;
            float length = std::sqrt(dx * dx + dy * dy);

            if (length > 0) {
                dx /= length;
                dy /= length;

                float px = -dy * radius;
                float py = dx * radius;

                GPoint corners[4] = {
                    {lastPoint.x + px, lastPoint.y + py},
                    {firstPoint.x + px, firstPoint.y + py},
                    {firstPoint.x - px, firstPoint.y - py},
                    {lastPoint.x - px, lastPoint.y - py}
                };

                builder.moveTo(corners[0]);
                builder.lineTo(corners[1]);
                builder.lineTo(corners[2]);
                builder.lineTo(corners[3]);
                builder.lineTo(corners[0]); // Explicitly close the cap

                // Add round cap at the start point
                float angleStep = M_PI / arcSegments;
                for (int j = 0; j <= arcSegments * 2; ++j) {
                    float theta = j * angleStep;
                    float x = firstPoint.x + radius * std::cos(theta);
                    float y = firstPoint.y + radius * std::sin(theta);
                    builder.lineTo(x, y);
                }
            }
        }

        return builder.detach();
    }

    // Override other methods (e.g., createVoronoiShader, createSweepGradient)
};

std::unique_ptr<GFinal> GCreateFinal() {
    return std::make_unique<MyFinal>();
}

*/