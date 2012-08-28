#include "Pixel.h"

Pixel::Pixel() :
    position(QPoint(0, 0)),
    gray(0) {
}

Pixel::Pixel(QPoint position, int gray) :
    position(position),
    gray(gray) {
}

QPoint Pixel::getPosition() {
  return position;
}

int Pixel::getGray() {
  return gray;
}

PixelVector* Pixel::binSort(PixelVector *original) {
  PixelVector *pixelGray[256];

  for (int i = 0; i < 256; ++i) {
    pixelGray[i] = new PixelVector();
  }

  for (PixelVector::iterator i = original->begin(), e = original->end(); i != e; ++i) {
    pixelGray[i->getGray()]->append(*i);
  }

  PixelVector *toReturn = pixelGray[0];
  for (int i = 1; i < 256; ++i) {
    (*toReturn) << *pixelGray[i];
    delete pixelGray[i];
  }

  return toReturn;
}
