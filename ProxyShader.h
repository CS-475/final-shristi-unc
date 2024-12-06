#ifndef PROXY_SHADER_H
#define PROXY_SHADER_H

#include "include/GShader.h"
#include "include/GMatrix.h"

class ProxyShader : public GShader {
    GShader* fRealShader;
    GMatrix fExtraTransform;

public:
    ProxyShader(GShader* shader, const GMatrix& extraTransform);

    bool isOpaque() override;
    bool setContext(const GMatrix& ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;
};

#endif
