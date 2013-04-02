#include "Region.h"

#include <assert.h>

using namespace std;

namespace mser {

  Region::Region() :
      gray(0),
      size(0),
      parent(0),
      cachedHigherGray(-1),
      cachedLowerGray(-1) {

    pixels = new PixelVector();
    children = new RegionSet();
    rootRegion = this;
  }

  Region::~Region() {
    delete pixels;
    delete children;
  }

  void Region::mergeInto(Region *other) {
    LOG4CXX_TRACE(logger, "Merging " << this << " into " << other);

    for (RegionSet::iterator i = children->begin(), e = children->end(); i != e; ++i) {
      (*i)->parent = other;
      other->children->insert(*i);
      (*i)->invalidateRootRegionCache();
    }

    for (PixelVector::iterator i = pixels->begin(), e = pixels->end(); i != e; ++i) {
      (*i)->region = other;
      other->pixels->append(*i);
    }

    other->size += size;
    assert(other->size >= 0);
  }

  void Region::groupUnder(Region *parent) {
    assert(this->parent == NULL);

    LOG4CXX_TRACE(logger, "Grouping " << this << " under " << parent);

    this->parent = parent;
    parent->children->insert(this);
    parent->size += size;
    assert(parent->size >= 0);
    invalidateRootRegionCache();
  }

  Region* Region::getRootRegion() {
    if (rootRegion == this) {
      if (parent != NULL) {
        rootRegion = parent;
      } else {
        return rootRegion;
      }
    }

    rootRegion = rootRegion->getRootRegion();

    return rootRegion;
  }

  PixelVector::iterator Region::pixelsBegin() {
    return pixels->begin();
  }

  PixelVector::iterator Region::pixelsEnd() {
    return pixels->end();
  }

  RegionSet* Region::exposeChildren() {
    return children;
  }

  void Region::invalidateRootRegionCache() {
    if (rootRegion != this) {
      rootRegion = this;

      LOG4CXX_TRACE(logger, "Invalidating child caches");

      for (RegionSet::iterator i = children->begin(), e = children->end(); i != e; ++i) {
        (*i)->invalidateRootRegionCache();
      }
    } else {
      LOG4CXX_TRACE(logger, "Not invalidating child caches");
    }
  }

  int Region::nextHigherGray() {
    if (cachedHigherGray < 0) {
      if (children->size() > 0) {
        int minHigherGray = 256;
        for (RegionSet::iterator i = children->begin(), e = children->end(); i != e; ++i) {
          minHigherGray = min(minHigherGray, (int) (*i)->gray);
        }
        cachedHigherGray = minHigherGray;
      } else {
        LOG4CXX_WARN(logger, "No higher gray found, using own gray!");
        // TODO(hermannloose): Revisit whether this is a good fallback.
        cachedHigherGray = gray;
      }
    }

    assert(cachedHigherGray >= 0);
    assert(cachedHigherGray < 256);

    return cachedHigherGray;
  }

  int Region::nextLowerGray() {
    if (cachedLowerGray < 0) {
      if (parent) {
        cachedLowerGray = parent->gray;
      } else {
        LOG4CXX_WARN(logger, "No lower gray found, using own gray!");
        // TODO(hermannloose): Revisit whether this is a good fallback.
        cachedLowerGray = gray;
      }
    }

    assert(cachedLowerGray >= 0);
    assert(cachedLowerGray < 256);

    return cachedLowerGray;
  }

}
