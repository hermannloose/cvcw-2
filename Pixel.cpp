#include "Pixel.h"
#include "Region.h"

#include <assert.h>
#include <iostream>

using namespace std;

namespace mser {

  Pixel::Pixel() :
      gray(0),
      region(0) {
  }

  Pixel::Pixel(QPoint position, int gray) :
      gray(gray),
      region(0) {

    x = position.x();
    y = position.y();
  }

  Pixel::Pixel(unsigned short x, unsigned short y, unsigned char gray) {
    this->x = x;
    this->y = y;
    this->gray = gray;
  }

  Region* Pixel::getRootRegion() {
    return region->getRootRegion();
  }

  PixelVector* Pixel::binSort(PixelVector *original) {
    PixelVector *pixelGray[256];

    for (int i = 0; i < 256; ++i) {
      pixelGray[i] = new PixelVector();
    }

    for (PixelVector::iterator i = original->begin(), e = original->end(); i != e; ++i) {
      pixelGray[(*i)->gray]->append(*i);
    }

    PixelVector *toReturn = pixelGray[0];
    for (int i = 1; i < 256; ++i) {
      (*toReturn) << *pixelGray[i];
      delete pixelGray[i];
    }

    return toReturn;
  }

  RegionSet* Pixel::getRootRegions(PixelVector *original) {
    RegionSet *rootRegions = new RegionSet();

    for (PixelVector::iterator i = original->begin(), e = original->end(); i != e; ++i) {
      rootRegions->insert((*i)->getRootRegion());
    }

    return rootRegions;
  }

}
