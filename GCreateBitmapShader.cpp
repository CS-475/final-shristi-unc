/*
 *  Copyright 2024 Shristi
 */

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GMath.h"
#include "my_utils.h"

class BitmapShader : public GShader {
public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode)
        : fBitmap(bitmap), fLocalMatrix(localMatrix), fTileMode(tileMode) {}

    bool isOpaque() override {
        return fBitmap.isOpaque();  // Check if all pixels in the bitmap are opaque
    }

    bool setContext(const GMatrix& ctm) override {
        // Combine the local matrix with the current transformation matrix (CTM)
        fCTM = GMatrix::Concat(ctm, fLocalMatrix);

        // Attempt to invert the combined matrix
        auto invCTM = fCTM.invert();
        if (!invCTM.has_value()) {
            // If inversion fails, return false and don't proceed with shading
            return false;
        }

        // Store the inverted matrix for shading use
        fInverseCTM = invCTM.value();
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            // Map the device space (x, y) to the bitmap space using the inverse CTM
            GPoint devicePoint = {static_cast<float>(x + i)+0.5, static_cast<float>(y)+0.5};
            GPoint bitmapPoint;
            fInverseCTM.mapPoints(&bitmapPoint, &devicePoint, 1);

            // int bx = static_cast<int>(bitmapPoint.x) % fBitmap.width();  // Wrap around for tiling
            // int by = static_cast<int>(bitmapPoint.y) % fBitmap.height();
            float bx, by;
            switch (fTileMode) {
                case GTileMode::kClamp:
                    bx = tileClamp(bitmapPoint.x, fBitmap.width() - 1);
                    by = tileClamp(bitmapPoint.y, fBitmap.height() - 1);
                    break;
                case GTileMode::kRepeat:
                    bx = tileRepeat(bitmapPoint.x, fBitmap.width());
                    by = tileRepeat(bitmapPoint.y, fBitmap.height());
                    break;
                case GTileMode::kMirror:
                    bx = tileMirror(bitmapPoint.x, fBitmap.width());
                    by = tileMirror(bitmapPoint.y, fBitmap.height());
                    break;
            }

            if (bx < 0) bx += fBitmap.width();  // Ensure positive indices
            if (by < 0) by += fBitmap.height();

            // Set the pixel color
            // row[i] = *fBitmap.getAddr(GRoundToInt(bx), GRoundToInt(by));
            row[i] = *fBitmap.getAddr(static_cast<int>(bx), static_cast<int>(by));

        }
    }

private:
    GBitmap fBitmap;
    GMatrix fLocalMatrix;
    GMatrix fCTM;          // Store the forward transformation
    GMatrix fInverseCTM;    // Store the inverse transformation
    GTileMode fTileMode;
};

// Factory function for creating the bitmap shader
std::shared_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GTileMode tileMode) {
    if (!bitmap.pixels()) {
        return nullptr;  // Return null if bitmap is invalid
    }
    return std::shared_ptr<GShader>(new BitmapShader(bitmap, localMatrix, tileMode));
}
