#pragma once

#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

  class Path;
  class RegionWalker;

  typedef QVector<Path*> PathVector;

  typedef QSet<RegionWalker*> WalkerSet;

  class MinimumResult {
    public:
      MinimumResult(mser::Region *region, double qi);
      mser::Region *region;
      double qi;
  };

  typedef QSet<MinimumResult*> ResultSet;

  class RegionWalker {

    public:
      RegionWalker(mser::Region *start, int delta);
      ~RegionWalker();
      ResultSet* findMSER();
      void ascend(int gray);
      int getSize();

      static log4cxx::LoggerPtr logger;

    private:
      int gray;
      int delta;
      mser::Region *current;
      RegionVector *regions;
      WalkerSet *walkers;
  };

  class Path {

    public:
      Path();
      Path(Path& other);
      ~Path();

      PathVector* descend();
      void ascend();
      double stability();

    private:
      RegionList *path;

      mser::Region *upper;
      unsigned int upperGray;

      mser::Region *current;
      unsigned int currentGray;

      mser::Region *lower;
      unsigned int lowerGray;
  };

}
