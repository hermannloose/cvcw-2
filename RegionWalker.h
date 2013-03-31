#pragma once

#include "Path.h"
#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

  class MinimumResult;
  class MinimumFinder;
  class Path;
  class RegionWalker;

  class MinimumResult {
    public:
      MinimumResult(mser::Region *region, double qi);
      mser::Region *region;
      double qi;
  };

  typedef QSet<MinimumResult*> ResultSet;

  typedef QList<MinimumFinder*> FinderList;

  class MinimumFinder {
    public:
      MinimumFinder(Path *path, mser::Region *region, double lastQi);

      bool hasMinimum();
      MinimumResult* getMinimum();

      FinderList* descend();

      static log4cxx::LoggerPtr logger;

    private:
      Path *path;
      // TODO(hermannloose): Make sure this is initialized.
      mser::Region *region;
      double lastQi;
      bool foundMinimum;
  };

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

}
