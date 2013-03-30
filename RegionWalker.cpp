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
      // TODO(hermannloose): Optimize for first child path.
      MinimumFinder *childFinder = new MinimumFinder(childPath, region, lastQi);
      double qi = childPath->stability();
      assert(qi >= 0);
      if (qi <= childFinder->lastQi) {
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
      if (region->gray > delta * 2 + 1) {
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
      paths->append(new Path(*i, 0, delta + 1, delta * 2 + 1));
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



  Path::Path(mser::Region *initialLeaf, unsigned int upperGray, unsigned int currentGray,
      unsigned int lowerGray) :
      upper(NULL),
      upperGray(upperGray),
      current(NULL),
      currentGray(currentGray),
      lower(NULL),
      lowerGray(lowerGray) {
    assert(initialLeaf->gray >= lowerGray);

    path = new RegionList();

    mser::Region *toInsert = initialLeaf;

    bool upperFound = false;
    bool currentFound = false;
    bool lowerFound = false;

    path->prepend(toInsert);

    // TODO(hermannloose): Can probably be made nicer.
    if (toInsert->gray <= upperGray) {
      upperFound = true;
      upper = toInsert;
    }

    if (toInsert->gray <= currentGray) {
      currentFound = true;
      current = toInsert;
    }

    if (toInsert->gray <= lowerGray) {
      lowerFound = true;
      lower = toInsert;
    }

    while (toInsert->parent) {
      toInsert = toInsert->parent;
      path->prepend(toInsert);

      if (!upperFound && toInsert->gray <= upperGray) {
        upperFound = true;
        upper = toInsert;
      }

      if (!currentFound && toInsert->gray <= currentGray) {
        currentFound = true;
        current = toInsert;
      }

      if (!lowerFound && toInsert->gray <= lowerGray) {
        lowerFound = true;
        lower = toInsert;
      }
    }

    // Images might not have completely black region to start with.
    if (upper == NULL) {
      upper = toInsert;
    }

    assert(upper);
    assert(current);
    assert(lower);
  }

  Path::Path(Path& other) :
      path(new RegionList(*other.path)),
      upper(other.upper),
      upperGray(other.upperGray),
      current(other.current),
      currentGray(other.currentGray),
      lower(other.lower),
      lowerGray(other.lowerGray) {
    assert(upper);
    assert(current);
    assert(lower);
  }

  Path::~Path() {
    delete path;
  }

  PathList* Path::descend() {
    PathList *childPaths = new PathList();

    assert(lowerGray < 255);

    // TODO(hermannloose): Add a method to check that before calling.
    assert(lower->children->size() > 0);

    if (path->indexOf(lower) < path->size() - 1) {
      // We have chosen a path before, use it.
      mser::Region *nextLower = path->at(path->indexOf(lower) + 1);
      if (nextLower->gray - lowerGray == 1) {
        lower = nextLower;
      }
      childPaths->append(this);
    } else {
      // We are at a branch or leaf.
      if (lower->nextHigherGray() - lowerGray == 1) {
        assert(lower->children->size() > 1);
        // Step down to next region.
        childPaths->reserve(lower->children->size());

        for (RegionSet::iterator i = lower->children->begin(), e = lower->children->end();
            i != e; ++i) {
          Path *childPath;
          mser::Region *childRegion = *i;

          if (childRegion == *lower->children->begin()) {
            childPath = this;
          } else {
            childPath = new Path(*this);
          }

          childPath->path->append(childRegion);

          if (childRegion->gray - lowerGray == 1) {
            childPath->lower = childRegion;
          }

          while (childRegion->children->size() == 1) {
            // Fill child path until next branch or leaf.
            childRegion = *childRegion->children->begin();
            childPath->path->append(childRegion);
          }

          childPaths->append(childPath);
        }
      } else {
        // Stay in this region.
        childPaths->append(this);
      }

      for (PathList::iterator i = childPaths->begin(), e = childPaths->end(); i != e; ++i) {
        Path *p = (*i);

        mser::Region *nextCurrent = p->path->at(p->path->indexOf(p->current) + 1);
        if (nextCurrent->gray - p->currentGray == 1) {
          // Step down to next region.
          p->current = nextCurrent;
        }

        mser::Region *nextUpper = p->path->at(p->path->indexOf(p->upper) + 1);
        if (nextUpper->gray - p->upperGray == 1) {
          // Step down to next region.
          p->upper = nextUpper;
          // TODO(hermannloose): Remove unneeded prefix of path.
        }

        ++p->lowerGray;
        ++p->currentGray;
        ++p->upperGray;
      }
    }

    return childPaths;
  }

  void Path::ascend() {
    assert(upperGray > 0);

    LOG4CXX_ERROR(logger, "This is unfinished and should not be called");

    QMutableListIterator<mser::Region*> i(*path);

    i.toFront();
    mser::Region *lastUpper = i.value();
    while (lastUpper->gray > upperGray) {
      assert(lastUpper->parent);

      i.insert(lastUpper->parent);
      i.toFront();

      lastUpper = i.value();
    }
    upper = lastUpper;

    i.toBack();
    while (i.value()->gray > lowerGray) {
      i.remove();
      i.toBack();
    }
    lower = i.value();

    while (i.value()->gray > currentGray) {
      i.previous();
    }

    current = i.value();
  }

  double Path::stability() {
    assert(upper->size >= 0);
    assert(current->size >= 0);
    assert(lower->size >= 0);

    assert(upper->size >= lower->size);

    return (upper->size - lower->size) / (double) current->size;
  }

  mser::Region* Path::currentRegion() {
    return current;
  }

  bool Path::atLeaf() {
    return lower->children->size() == 0;
  }

}
