#pragma once

#include "Globals.h"

#include <log4cxx/logger.h>

#include <QPoint>

namespace mser {

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

}
