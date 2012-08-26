#include "QuadTree.h"

#include <QPoint>
#include <QRect>

#include <iostream>

using namespace std;

QuadTree::QuadTree(QRect *region, unsigned capacity) {
  this->region = region;

  points = new PointVector();
  points->reserve(capacity);
  this->capacity = capacity;

  isLeaf = true;
}

bool QuadTree::insert(QPoint point) {
  if (!region->contains(point)) {
    return false;
  }

  if (isLeaf) {
    if ((points->size() < capacity) || (capacity == 0)) {
      cerr << "Inserting (" << point.x() << ", " << point.y() << ") in "
          << "[" << region->left() << ", " << region->top() << ", " << region->right() << ", "
          << region->bottom() << "]." << endl;

      points->append(point);
      return true;
    } else {
      subdivide();
      return this->insert(point);
    }
  } else {
    if (northWest->insert(point)) { return true; }
    if (northEast->insert(point)) { return true; }
    if (southWest->insert(point)) { return true; }
    if (southEast->insert(point)) { return true; }
  }

  // This should never happen.
  return false;
}

PointVector* QuadTree::queryRange(QRect range) {
  PointVector *inRange = new PointVector();

  if (!region->intersects(range)) {
    return inRange;
  }

  if (isLeaf) {
    for (PointVector::iterator i = points->begin(), e = points->end(); i != e; ++i) {
      if (range.contains(*i)) {
        inRange->append(*i);
      }
    }
  } else {
    (*inRange) << *(northWest->queryRange(range));
    (*inRange) << *(northEast->queryRange(range));
    (*inRange) << *(southWest->queryRange(range));
    (*inRange) << *(southEast->queryRange(range));
  }

  return inRange;
}

void QuadTree::dump() {
  cerr << "[" << region->left() << ", " << region->top() << ", " << region->right() << ", "
      << region->bottom() << "]." << endl;
}

void QuadTree::subdivide() {
  cerr << "Subdividing ";
  dump();

  if ((region->width() < 2) && (region->height() < 2)) {
    cerr << "Region can't be divided anymore, setting to unlimited capacity." << endl;
    capacity = 0;
    return;
  }


  isLeaf = false;

  QPoint center = region->center();
  unsigned splitWidth = region->width() / 2;
  unsigned splitHeight = region->height() / 2;

  northWest = new QuadTree(
      new QRect(region->left(), region->top(), splitWidth, splitHeight), capacity);

  cerr << "NW ";
  northWest->dump();

  northEast = new QuadTree(
      new QRect(center.x() + 1, region->top(), splitWidth, splitHeight), capacity);

  cerr << "NE ";
  northEast->dump();

  southWest = new QuadTree(
      new QRect(region->left(), center.y() + 1, splitWidth, splitHeight), capacity);

  cerr << "SW ";
  southWest->dump();

  southEast = new QuadTree(
      new QRect(center.x() + 1, center.y() + 1, splitWidth, splitHeight), capacity);

  cerr << "SE ";
  southEast->dump();

  for (PointVector::iterator i = points->begin(), e = points->end(); i != e; ++i) {
    this->insert(*i);
  }
  delete points;
}
