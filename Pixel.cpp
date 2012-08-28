#include "Pixel.h"

#include <assert.h>
#include <iostream>

using namespace std;

namespace mser {

  Pixel::Pixel() :
      position(QPoint(0, 0)),
      gray(0),
      region(0) {
  }

  Pixel::Pixel(QPoint position, int gray) :
      position(position),
      gray(gray),
      region(0) {

  }

  Region* Pixel::getRootRegion() {
    Region *rootRegion = region;
    assert(rootRegion);
    while (rootRegion->parent != NULL) {
      rootRegion = rootRegion->parent;
    }

    return rootRegion;
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

  Region::Region() :
      gray(0),
      parent(0) {

    pixels = new PixelVector();
    children = new RegionSet();
  }

}
