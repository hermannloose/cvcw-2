#include "RegionWalker.h"

#include <log4cxx/logger.h>

#include <QMutableVectorIterator>

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

}
