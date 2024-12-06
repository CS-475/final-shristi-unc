// Adding a comment
#include "CompositeShader.h"
#include "my_utils.h"

CompositeShader::CompositeShader(std::shared_ptr<GShader> shader1, std::shared_ptr<GShader> shader2)
    : fShader1(std::move(shader1)), fShader2(std::move(shader2)) {}
   
bool CompositeShader::isOpaque() {
    return fShader1->isOpaque() && fShader2->isOpaque();
}

bool CompositeShader::setContext(const GMatrix& ctm) {
    return fShader1->setContext(ctm) && fShader2->setContext(ctm);
}

void CompositeShader::shadeRow(int x, int y, int count, GPixel row[]) {
    GPixel row1[count], row2[count];
    fShader1->shadeRow(x, y, count, row1);
    fShader2->shadeRow(x, y, count, row2);

    for (int i = 0; i < count; ++i) {
        int a = divide255(GET_ALPHA(row1[i]) * GET_ALPHA(row2[i]));
        int r = divide255(GET_RED(row1[i]) * GET_RED(row2[i]));
        int g = divide255(GET_GREEN(row1[i]) * GET_GREEN(row2[i]));
        int b = divide255(GET_BLUE(row1[i]) * GET_BLUE(row2[i]));

        row[i] = GPixel_PackARGB(a, r, g, b);
    }
}
