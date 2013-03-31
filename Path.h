#pragma once

#include "Globals.h"
#include "Pixel.h"

#include <log4cxx/logger.h>

namespace mser {

  typedef QList<Path*> PathList;

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

      friend class MinimumFinder;

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
