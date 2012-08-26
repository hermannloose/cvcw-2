#pragma once

#include <QPoint>
#include <QRect>
#include <QVector>

typedef QVector<QPoint> PointVector;

class QuadTree {

  public:
    QuadTree(QRect *region, unsigned capacity);

    bool insert(QPoint point);
    PointVector* queryRange(QRect range);

  private:
    QRect *region;
    PointVector *points;
    unsigned capacity;
    bool isLeaf;

    QuadTree *northWest;
    QuadTree *northEast;
    QuadTree *southWest;
    QuadTree *southEast;

    void dump();
    void subdivide();
};
