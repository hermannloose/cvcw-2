#pragma once

#include "Pixel.h"

namespace mser {

  class RegionWalker;

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

    private:
      int gray;
      int delta;
      mser::Region *current;
      RegionVector *regions;
      WalkerSet *walkers;
  };

}