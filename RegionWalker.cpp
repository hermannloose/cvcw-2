#include "RegionWalker.h"

#include <log4cxx/logger.h>

#include <QMutableVectorIterator>

#include <assert.h>
#include <iostream>

using namespace log4cxx;

using namespace std;

namespace mser {

  MinimumResult::MinimumResult(mser::Region *region, double qi) :
      region(region),
      qi(qi) {
  }

  RegionWalker::RegionWalker(mser::Region *start, int delta) :
      start(start), delta(delta) {
  }

  RegionWalker::~RegionWalker() {
  }

  ResultSet* RegionWalker::findMSER() {
    ResultSet *results = new ResultSet();

    return results;
  }



  Path::Path(mser::Region *initialLeaf, unsigned int upperGray, unsigned int currentGray,
      unsigned int lowerGray) :
      upperGray(upperGray), currentGray(currentGray), lowerGray(lowerGray) {
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
  }

  Path::Path(Path& other) {
    path = new RegionList(*other.path);

    upper = other.upper;
    upperGray = other.upperGray;

    current = other.current;
    currentGray = other.currentGray;

    lower = other.lower;
    lowerGray = other.lowerGray;
  }

  Path::~Path() {
    delete path;
  }

  PathVector* Path::descend() {
    PathVector *childPaths = new PathVector();

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
      // We are at a branch.
      assert(lower->children->size() > 1);

      if (lower->nextHigherGray() - lowerGray == 1) {
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
            // Fill child path until next branch.
            childRegion = *childRegion->children->begin();
            childPath->path->append(childRegion);
          }

          childPaths->append(childPath);
        }
      } else {
        // Stay in this region.
        childPaths->append(this);
      }

      for (PathVector::iterator i = childPaths->begin(), e = childPaths->end(); i != e; ++i) {
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
    return (upper->size - lower->size) / (double) current->size;
  }

}
