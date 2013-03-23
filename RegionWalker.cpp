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

  RegionWalker::RegionWalker(mser::Region *start, int delta) {
    current = start;
    regions = new RegionVector();
    regions->append(start);
    this->gray = start->gray;
    this->delta = delta;
  }

  RegionWalker::~RegionWalker() {
    delete regions;
  }

  void RegionWalker::ascend(int gray) {
    QMutableVectorIterator<Region*> i(*regions);
    while (i.hasNext()) {
      Region *current = i.next();
      if (current->gray < gray) {
        i.remove();
        i.toBack();
        for (RegionSet::iterator ci = current->children->begin(), ce = current->children->end();
            ci != ce; ++ci) {
          i.insert(*ci);
        }
        i.toFront();
      }
    }
  }

  int RegionWalker::getSize() {
    int size = 0;
    for (RegionVector::iterator i = regions->begin(), e = regions->end(); i != e; ++i) {
      size += (*i)->size;
    }
    return size;
  }

  ResultSet* RegionWalker::findMSER() {
    ResultSet *results = new ResultSet();

    double lastQi = 20;
    bool lastWasMin = false;
    double qi = 0;

    while (current->children->size() == 1) {
      mser::Region *parent = current;
      while (parent->parent && parent->parent->gray < gray - delta) {
        parent = parent->parent;
      }
      ascend(gray + delta);
      qi = ((double) (parent->size - getSize()) / (double) current->size);
      LOG4CXX_TRACE(logger, qi);
      if (qi <= lastQi - 0.1) {
        lastWasMin = true;
      } else {
        LOG4CXX_DEBUG(logger, "Found minimum");
        results->insert(new MinimumResult(current->parent, lastQi));
        lastWasMin = false;
      }
      lastQi = qi;

      current = *(current->children->begin());
      gray = current->gray;
    }

    if (!current->children->isEmpty()) {
      // Spawn child walkers.
      for (RegionSet::iterator i = current->children->begin(), e = current->children->end();
          i != e; ++i) {
        RegionWalker *childWalker = new RegionWalker(*i, delta);
        ResultSet *childResults = childWalker->findMSER();
        results->unite(*childResults);
        delete childWalker;
        delete childResults;
      }
    }

    LOG4CXX_TRACE(logger, "Returning results");
    return results;
  }



  Path::Path() {
    path = new RegionList();
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
