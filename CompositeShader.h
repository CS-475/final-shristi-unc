#include "include/GShader.h"
#include <vector>
#include <memory>

#ifndef COMPOSITE_SHADER_H
#define COMPOSITE_SHADER_H

class CompositeShader : public GShader {
    std::shared_ptr<GShader> fShader1;
    std::shared_ptr<GShader> fShader2;

public:
    CompositeShader(std::shared_ptr<GShader> shader1, std::shared_ptr<GShader> shader2);

    bool isOpaque() override;
    bool setContext(const GMatrix& ctm) override;
    void shadeRow(int x, int y, int count, GPixel row[]) override;

};

#endif