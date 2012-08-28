#pragma once

#include <QPoint>
#include <QSet>
#include <QVector>

namespace mser {

  class Pixel;
  class Region;

  typedef QVector<Pixel*> PixelVector;
  typedef QSet<Pixel*> PixelSet;

  typedef QSet<Region*> RegionSet;

  class Pixel {

    public:
      Pixel();
      Pixel(QPoint position, int gray);

      QPoint position;
      int gray;

      Region *region;

      Region* getRootRegion();

      static PixelVector* binSort(PixelVector *original);
      static RegionSet* getRootRegions(PixelVector *original);
  };

  class Region {
    public:
      Region();

      int gray;
      PixelVector *pixels;

      Region *parent;
      RegionSet *children;
  };

}
