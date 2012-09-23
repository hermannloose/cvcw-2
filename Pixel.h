#pragma once

#include <QPoint>
#include <QSet>
#include <QVector>

namespace mser {

  class Pixel;
  class Region;

  typedef QVector<Pixel*> PixelVector;
  typedef QSet<Pixel*> PixelSet;

  typedef QVector<Region*> RegionVector;
  typedef QSet<Region*> RegionSet;

  class Pixel {

    public:
      Pixel();
      Pixel(QPoint position, int gray);

      unsigned short x;
      unsigned short y;
      unsigned char gray;

      Region *region;

      Region* getRootRegion();

      static PixelVector* binSort(PixelVector *original);
      static RegionSet* getRootRegions(PixelVector *original);
  };

  class Region {
    public:
      Region();
      ~Region();

      unsigned char gray;
      PixelVector *pixels;
      int size;

      Region *parent;
      RegionSet *children;
  };

}
