#include "RegionWalker.h"

#include <log4cxx/logger.h>

#include <QMutableVectorIterator>

#include <assert.h>
#include <climits>
#include <iostream>

using namespace log4cxx;

using namespace std;

namespace mser {

  MinimumResult::MinimumResult(mser::Region *region, double qi) :
      region(region),
      qi(qi) {
  }



  MinimumFinder::MinimumFinder(Path *path, mser::Region *region, double lastQi) :
      path(path), region(region), lastQi(lastQi), foundMinimum(false) {

  }

  bool MinimumFinder::hasMinimum() {
    return foundMinimum;
  }

  MinimumResult* MinimumFinder::getMinimum() {
    return new MinimumResult(region, lastQi);
  }

  FinderList* MinimumFinder::descend() {
    assert(!foundMinimum);

    FinderList *childFinders = new FinderList();

    // TODO(hermannloose): Clean up control flow.
    if (path->atLeaf()) {
      this->foundMinimum = true;

      LOG4CXX_DEBUG(logger, "Terminating at leaf with " << lastQi);

      childFinders->append(this);

      return childFinders;
    }

    PathList *childPaths = path->descend();

    assert(childPaths->size() > 0);

    for (PathList::iterator i = childPaths->begin(), e = childPaths->end(); i != e; ++i) {
      Path *childPath = *i;

      LOG4CXX_TRACE(logger, "Path (" << childPath->upperGray << "/" << childPath->currentGray
          << "/" << childPath->lowerGray << ")");

      // TODO(hermannloose): Optimize for first child path.
      MinimumFinder *childFinder = new MinimumFinder(childPath, region, lastQi);
      double qi = childPath->stability();
      assert(qi >= 0);
      if (qi <= childFinder->lastQi - 0.01) {
        childFinder->region = childPath->currentRegion();
        childFinder->lastQi = qi;

        LOG4CXX_TRACE(logger, "Region " << region << ", qi: " << qi);
      } else {
        childFinder->foundMinimum = true;

        LOG4CXX_DEBUG(logger, "Found minimum of " << qi);
      }

      childFinders->append(childFinder);
    }

    delete childPaths;

    LOG4CXX_TRACE(logger, childFinders->size() << " child finders");

    return childFinders;
  }



  RegionWalker::RegionWalker(mser::Region *start, int delta) :
      start(start), delta(delta) {
  }

  RegionWalker::~RegionWalker() {
  }

  ResultSet* RegionWalker::findMSER() {
    RegionSet *initialLeaves = new RegionSet();
    RegionList *toProcess = new RegionList();
    toProcess->append(start);

    while (!toProcess->isEmpty()) {
      mser::Region *region = toProcess->takeFirst();
      if (region->gray >= min(delta * 2, 255)) {
        initialLeaves->insert(region);
      } else {
        if (!region->children->isEmpty()) {
          for (RegionSet::iterator i = region->children->begin(), e = region->children->end();
              i != e; ++i) {
            toProcess->append(*i);
          }
        }
      }
    }

    delete toProcess;

    PathList *paths = new PathList();
    for (RegionSet::iterator i = initialLeaves->begin(), e = initialLeaves->end(); i != e; ++i) {
      paths->append(new Path(*i, 0, min(delta, 255), min(delta * 2, 255)));
    }

    // TODO(hermannloose): Change to DEBUG level.
    LOG4CXX_INFO(logger, "Found " << paths->size() << " initial paths");

    FinderList *finders = new FinderList();
    for (PathList::iterator i = paths->begin(), e = paths->end(); i != e; ++i) {
      Path *path = *i;
      finders->append(new MinimumFinder(path, path->currentRegion(), (double) INT_MAX));
    }

    delete paths;

    ResultSet *results = new ResultSet();

    while (!finders->isEmpty()) {
      LOG4CXX_TRACE(logger, finders->size() << " finders");

      MinimumFinder *finder = finders->takeFirst();
      if (finder->hasMinimum()) {
        results->insert(finder->getMinimum());

        LOG4CXX_DEBUG(logger, "Found minimum");
      } else {
        LOG4CXX_TRACE(logger, "Descending");

        FinderList* childFinders = finder->descend();
        finders->append(*childFinders);
        delete childFinders;
      }
    }

    LOG4CXX_INFO(logger, "Found " << results->size() << " MSERs");

    return results;
  }

}
