#pragma once

#include "Pixel.h"

#include <QVector>

namespace mser {

  typedef QVector<PixelVector*> ColumnVector;

  class PixelImage {
    public:
      PixelImage(int width, int height);
      void insert(Pixel *pixel);
      PixelVector* neighbours(Pixel *pixel);

    private:
      int width;
      int height;
      ColumnVector *columns;
  };

}
