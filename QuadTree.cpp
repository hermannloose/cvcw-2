#include "QuadTree.h"

#include <QPoint>
#include <QRect>

#include <iostream>

using namespace std;

namespace mser {

  QuadTree::QuadTree(QRect *region, unsigned capacity, unsigned minDimension) {
    this->region = region;

    points = new PixelVector();
    points->reserve(capacity);
    this->capacity = capacity;
    this->minDimension = minDimension;

    isLeaf = true;
  }

  bool QuadTree::insert(Pixel *pixel) {
    if (!region->contains(pixel->position)) {
      return false;
    }

    if (isLeaf) {
      if ((points->size() < capacity) || (capacity == 0)) {
        /*
        cerr << "Inserting (" << pixel->getPosition().x() << ", " << pixel->getPosition().y()
            << ") in " << "[" << region->left() << ", " << region->top() << ", " << region->right()
            << ", " << region->bottom() << "]." << endl;
            */

        points->append(pixel);
        return true;
      } else {
        subdivide();
        return this->insert(pixel);
      }
    } else {
      if (northWest->insert(pixel)) { return true; }
      if (northEast->insert(pixel)) { return true; }
      if (southWest->insert(pixel)) { return true; }
      if (southEast->insert(pixel)) { return true; }
    }

    // This should never happen.
    return false;
  }

  PixelVector* QuadTree::queryRange(QRect *range) {
    PixelVector *inRange = new PixelVector();

    if (!region->intersects(*range)) {
      return inRange;
    }

    if (isLeaf) {
      for (PixelVector::iterator i = points->begin(), e = points->end(); i != e; ++i) {
        if (range->contains((*i)->position)) {
          inRange->append(*i);
        }
      }
    } else {
      PixelVector *nwRange = northWest->queryRange(range);
      if (!nwRange->isEmpty()) {
        (*inRange) << *nwRange;
      }

      PixelVector *neRange = northEast->queryRange(range);
      if (!nwRange->isEmpty()) {
        (*inRange) << *neRange;
      }

      PixelVector *swRange = southWest->queryRange(range);
      if (!nwRange->isEmpty()) {
        (*inRange) << *swRange;
      }

      PixelVector *seRange = southEast->queryRange(range);
      if (!nwRange->isEmpty()) {
        (*inRange) << *seRange;
      }

      /*
      (*inRange) << *(northWest->queryRange(range));
      (*inRange) << *(northEast->queryRange(range));
      (*inRange) << *(southWest->queryRange(range));
      (*inRange) << *(southEast->queryRange(range));
      */
    }

    return inRange;
  }

  void QuadTree::dump() {
    cerr << "[" << region->left() << ", " << region->top() << ", " << region->right() << ", "
        << region->bottom() << "]." << endl;
  }

  void QuadTree::subdivide() {
    /*
    cerr << "Subdividing ";
    dump();
    */

    if ((region->width() <= minDimension) && (region->height() <= minDimension)) {
      /*
      cerr << "Region can't be divided anymore, setting to unlimited capacity." << endl;
      */
      capacity = 0;
      return;
    }


    isLeaf = false;

    QPoint center = region->center();
    unsigned splitWidth = region->width() / 2;
    unsigned splitHeight = region->height() / 2;

    northWest = new QuadTree(
        new QRect(region->left(), region->top(), splitWidth, splitHeight), capacity,
        minDimension);

    /*
    cerr << "NW ";
    northWest->dump();
    */

    northEast = new QuadTree(
        new QRect(center.x() + 1, region->top(), splitWidth, splitHeight), capacity,
        minDimension);

    /*
    cerr << "NE ";
    northEast->dump();
    */

    southWest = new QuadTree(
        new QRect(region->left(), center.y() + 1, splitWidth, splitHeight), capacity,
        minDimension);

    /*
    cerr << "SW ";
    southWest->dump();
    */

    southEast = new QuadTree(
        new QRect(center.x() + 1, center.y() + 1, splitWidth, splitHeight), capacity,
        minDimension);

    /*
    cerr << "SE ";
    southEast->dump();
    */

    for (PixelVector::iterator i = points->begin(), e = points->end(); i != e; ++i) {
      this->insert(*i);
    }
    delete points;
  }

}
