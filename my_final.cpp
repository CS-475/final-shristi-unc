#include "include/GFinal.h"
#include "include/GPathBuilder.h"
#include "include/GPath.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "include/GShader.h"
#include "my_utils.h"
#include <memory>
#include <cmath>
#include <vector>

class MyFinal : public GFinal {
public:
    // Override the strokePolygon method
    std::shared_ptr<GPath> strokePolygon(const GPoint points[], int count, float width, bool isClosed) override {
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

        // Detach and return the constructed path
        return builder.detach();
    }    

    // Override the createLinearPosGradient method
    std::shared_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1, const GColor colors[], const float pos[], int count) override {
        if (count < 2 || !colors || !pos) {
            return nullptr;
        }

        // Interpolate based on the pos array
        std::vector<GColor> interpolatedColors;
        std::vector<float> interpolatedPositions;

        // Iterate over each segment between positions
        for (int i = 0; i < count - 1; ++i) {
            float startPos = pos[i];
            float endPos = pos[i + 1];
            int steps = static_cast<int>((endPos - startPos) * 100); // Finer steps for smooth gradients

            for (int j = 0; j <= steps; ++j) {
                float t = static_cast<float>(j) / steps; // Interpolation parameter between 0 and 1

                // Interpolate the position
                float interpolatedPos = startPos * (1 - t) + endPos * t;
                interpolatedPositions.push_back(interpolatedPos);

                // Interpolate the colors
                GColor c0 = colors[i];
                GColor c1 = colors[i + 1];
                GColor interpolatedColor = {
                    c0.r * (1 - t) + c1.r * t,
                    c0.g * (1 - t) + c1.g * t,
                    c0.b * (1 - t) + c1.b * t,
                    c0.a * (1 - t) + c1.a * t
                };

                interpolatedColors.push_back(interpolatedColor);
            }
        }

        // Use GCreateLinearGradient with the newly interpolated colors and positions
        return GCreateLinearGradient(p0, p1, interpolatedColors.data(), interpolatedColors.size(), GTileMode::kClamp);
    }

   
}; 

std::unique_ptr<GFinal> GCreateFinal() {
    return std::make_unique<MyFinal>();
}

