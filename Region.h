#pragma once

#include "Globals.h"
#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

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
