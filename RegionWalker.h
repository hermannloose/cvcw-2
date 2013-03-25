#pragma once

#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

  class Path;
  class RegionWalker;

  typedef QVector<Path*> PathVector;

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

      static log4cxx::LoggerPtr logger;

    private:
      mser::Region *start;
      int delta;
  };

  class Path {

    public:
      Path(mser::Region *initialLeaf, unsigned int upperGray, unsigned int currentGray,
          unsigned int lowerGray);
      Path(Path& other);
      ~Path();

      PathVector* descend();
      void ascend();
      double stability();

      static log4cxx::LoggerPtr logger;

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
