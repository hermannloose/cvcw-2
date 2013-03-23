#pragma once

#include <log4cxx/logger.h>

#include <QList>
#include <QPoint>
#include <QSet>
#include <QVector>

namespace mser {

  class Path;
  class Pixel;
  class Region;
  class RegionWalker;

  typedef QVector<Pixel*> PixelVector;
  typedef QSet<Pixel*> PixelSet;

  typedef QList<Region*> RegionList;
  typedef QVector<Region*> RegionVector;
  typedef QSet<Region*> RegionSet;

  class Pixel {

    public:
      Pixel();
      Pixel(QPoint position, int gray);
      Pixel(unsigned short x, unsigned short y, unsigned char gray);

      unsigned short x;
      unsigned short y;
      unsigned char gray;

      Region *region;

      Region* getRootRegion();

      static PixelVector* binSort(PixelVector *original);
      static RegionSet* getRootRegions(PixelVector *original);
  };

  class Region {
    friend class Path;
    friend class RegionWalker;

    public:
      Region();
      ~Region();

      unsigned char gray;
      PixelVector *pixels;
      int size;

      void mergeInto(Region *other);
      void groupUnder(Region *parent);
      Region* getRootRegion();

      // TODO(hermannloose): Hack, remove.
      PixelVector::iterator pixelsBegin();
      PixelVector::iterator pixelsEnd();
      RegionSet* exposeChildren();

      static log4cxx::LoggerPtr logger;

    private:
      Region *parent;
      RegionSet *children;
      Region *rootRegion;

      int cachedHigherGray;
      int cachedLowerGray;

      void invalidateRootRegionCache();

      int nextHigherGray();
      int nextLowerGray();
  };

}
