#pragma once

#include "Pixel.h"

#include <QPoint>
#include <QRect>
#include <QVector>

namespace mser {

  class QuadTree {

    public:
      QuadTree(QRect *region, unsigned capacity, unsigned minDimension);

      bool insert(Pixel *pixel);
      PixelVector* queryRange(QRect *range);

    private:
      QRect *region;
      PixelVector *points;
      // Pixels per region / bucket.
      unsigned capacity;
      // Don't split below this size.
      unsigned minDimension;
      bool isLeaf;

      QuadTree *northWest;
      QuadTree *northEast;
      QuadTree *southWest;
      QuadTree *southEast;

      void dump();
      void subdivide();
  };

}
