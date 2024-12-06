#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "include/GColor.h"
#include "my_utils.h"
#include <vector>
#include <cmath>

class LinearGradientShader : public GShader {
public:
    LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode)
        : fP0(p0), fP1(p1), fColors(colors, colors + count), fColorCount(count), fTileMode(tileMode) {}

    bool isOpaque() override {
        // Check if all colors are opaque
        for (const auto& color : fColors) {
            if (color.a < 1.0f) {
                return false;
            }
        }
        return true;
    }

   bool setContext(const GMatrix& ctm) override {
        GPoint u = fP1 - fP0; // vector
        GPoint v = {u.y, -u.x}; // perp vector
        // origin = p0
        GMatrix gradientMatrix = GMatrix(u.x, v.x, fP0.x,
                                 u.y, v.y, fP0.y);

        // Combine with the CTM and invert
        auto invCTM = GMatrix::Concat(ctm, gradientMatrix).invert();
        if (!invCTM.has_value()) {
            return false;  // Return false if the matrix can't be inverted
        }

        fInverseMatrix = invCTM.value();
        return true;
    }

   void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            GPoint pt = { static_cast<float>(x + i)+0.5, static_cast<float>(y)+0.5 };
            fInverseMatrix.mapPoints(&pt, &pt, 1);

            float t = pt.x;  // Use transformed x coordinate as 't' for the gradient

            switch (fTileMode) {
                case GTileMode::kClamp:
                    t = std::max(0.0f, std::min(t, 1.0f));
                    break;
                case GTileMode::kRepeat:
                    t = tileRepeat(t, 1.0f);
                    break;
                case GTileMode::kMirror:
                    t = tileMirror(t, 1.0f);
                    break;
            }

            // Interpolate between the colors based on t
            int index = static_cast<int>(t * (fColorCount - 1));
            float localT = (t * (fColorCount - 1)) - index;

           // Ensure index doesn't exceed bounds
            GColor c0 = fColors[index];
            GColor c1 = fColors[std::min(index + 1, fColorCount - 1)];

            GColor color = {
                c0.r * (1 - localT) + c1.r * localT,
                c0.g * (1 - localT) + c1.g * localT,
                c0.b * (1 - localT) + c1.b * localT,
                c0.a * (1 - localT) + c1.a * localT
            };

            // Premultiply the color and convert it to a pixel
            row[i] = ColorToPixel(color.pinToUnit());
        }
    }


private:
    GPoint fP0, fP1;
    std::vector<GColor> fColors;
    int fColorCount;
    GMatrix fLocalMatrix;
    GMatrix fInverseMatrix;
    GMatrix fCTM;
    GTileMode fTileMode;
};

// Factory function to create the linear gradient shader
std::shared_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tileMode) {
    if (count < 1) {
        return nullptr;
    }
    return std::make_shared<LinearGradientShader>(p0, p1, colors, count, tileMode);
}