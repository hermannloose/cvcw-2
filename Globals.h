#pragma once

#include <QList>
#include <QSet>
#include <QVector>

namespace mser {

  class MinimumFinder;
  class MinimumResult;
  class Path;
  class Pixel;
  class PixelImage;
  class Region;
  class RegionWalker;

  typedef QList<Path*> PathList;

  typedef QSet<Pixel*> PixelSet;
  typedef QVector<Pixel*> PixelVector;

  typedef QList<Region*> RegionList;
  typedef QVector<Region*> RegionVector;
  typedef QSet<Region*> RegionSet;

}
