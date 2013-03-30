#pragma once

#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

  class MinimumResult;
  class MinimumFinder;
  class Path;
  class RegionWalker;

  typedef QList<Path*> PathList;
  typedef QVector<Path*> PathVector;

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

  class Path {

    public:
      Path(mser::Region *initialLeaf, unsigned int upperGray, unsigned int currentGray,
          unsigned int lowerGray);
      Path(Path& other);
      ~Path();

      PathList* descend();
      void ascend();
      double stability();
      mser::Region* currentRegion();

      bool atLeaf();

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
