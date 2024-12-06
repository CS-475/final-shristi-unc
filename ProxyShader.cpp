#include "ProxyShader.h"


ProxyShader::ProxyShader(GShader* shader, const GMatrix& extraTransform)
   : fRealShader(shader), fExtraTransform(extraTransform) {}


bool ProxyShader::isOpaque() {
   return fRealShader->isOpaque();
}


bool ProxyShader::setContext(const GMatrix& ctm) {
   // Apply the extra transformation
   GMatrix combinedMatrix = ctm * fExtraTransform;

   return fRealShader->setContext(combinedMatrix);
}

void ProxyShader::shadeRow(int x, int y, int count, GPixel row[]) {
   // Delegate shading to the real shader
   fRealShader->shadeRow(x, y, count, row);
}