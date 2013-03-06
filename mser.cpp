#include "Pixel.h"
#include "PixelImage.h"
#include "RegionWalker.h"

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QRgb>
#include <QStringBuilder>

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mser;

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

static char *opts = "d:";

RegionSet* findLowers(mser::Region *region, int depth);

RegionSet* walkRegions(mser::Region *current, mser::Region *lower, mser::Region *upper,
    double minqi, bool lastWasMin);

void paintRegion(mser::Region *region, QImage *output);

void dumpRegions(mser::Region *region, int indent, int depth);

void mergeRegions(mser::Region *merge, mser::Region *into);

LoggerPtr logger(Logger::getLogger("mser"));
LoggerPtr RegionWalker::logger(Logger::getLogger("mser.RegionWalker"));

int main(int argc, char *argv[]) {
  short delta = 5;
  string *inputFilename;
  string *outputFilename;

  char c;
  while ((c = getopt(argc, argv, opts)) != -1) {
    switch (c) {
      case 'd':
        delta = atoi(optarg);
        break;
      default:
        break;
    }
  }

  if (optind < argc) {
    inputFilename = new string(argv[optind]);
  } else {
    inputFilename = new string("input.png");
  }

  ++optind;

  if (optind < argc) {
    outputFilename = new string(argv[optind]);
  } else {
    outputFilename = new string("output.png");
  }

  ++optind;

  try {
    if (optind < argc) {
      PropertyConfigurator::configure(argv[optind]);
    } else {
      BasicConfigurator::configure();
    }
  } catch (Exception&) {
  }

  QImage input(inputFilename->c_str());

  int width = input.width();
  int height = input.height();

  LOG4CXX_INFO(logger, "Got image [" << width << "x" << height << "]");
  LOG4CXX_INFO(logger, "Using delta of " << delta);

  PixelVector *pixels = new PixelVector();
  pixels->reserve(width * height);

  LOG4CXX_INFO(logger, "Building pixels");

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      pixels->append(new Pixel(x, y, qGray(input.pixel(x, y))));
    }
  }

  LOG4CXX_INFO(logger, "Sorting pixels");

  PixelVector *sortedPixels = Pixel::binSort(pixels);
  delete pixels;
  reverse(sortedPixels->begin(), sortedPixels->end());

  LOG4CXX_INFO(logger, "Placing pixels");

  PixelImage *pimage = new PixelImage(width, height);
  RegionSet *regionLeaves = new RegionSet();

  int pixelsWithNeighbours = 0;
  for (PixelVector::iterator i = sortedPixels->begin(), e = sortedPixels->end(); i != e; ++i) {
    Pixel *pixel = *i;

    PixelVector *neighbours = pimage->neighbours(*i);

    if (neighbours->isEmpty()) {
      // Start a new region.
      mser::Region *region = new mser::Region();
      region->gray = pixel->gray;
      pixel->region = region;
      region->pixels->append(pixel);
      region->size = 1;
    } else {
      RegionSet *rootRegions = Pixel::getRootRegions(neighbours);

      // Speculative new region.
      mser::Region *region = new mser::Region();
      region->gray = pixel->gray;

      for (RegionSet::iterator ri = rootRegions->begin(), re = rootRegions->end();
          ri != re; ++ri) {

        if ((*ri)->gray > pixel->gray) {
          // Brighter regions to be grouped under this one.

          // HACK.
          mser::Region *parentToSet = region;
          /*
          for (int d = pixel->gray; d < (*ri)->gray; ++d) {
            mser::Region *dummyRegion = new mser::Region();
            dummyRegion->parent = parentToSet;
            dummyRegion->size = (*ri)->size;
            parentToSet->children->insert(dummyRegion);
            parentToSet = dummyRegion;
          }
          */

          (*ri)->parent = parentToSet;
          parentToSet->children->insert(*ri);
          region->size += (*ri)->size;
          regionLeaves->remove(*ri);
        } else {
          assert((*ri)->gray == pixel->gray);
          // We've found a preexisting region for the same threshold, merge.
          mergeRegions(region, *ri);

          regionLeaves->remove(region);
          delete region;

          region = *ri;
        }
      }

      pixel->region = region;
      region->pixels->append(pixel);
      region->size += 1;
      regionLeaves->insert(region);

      delete rootRegions;
    }

    pimage->insert(pixel);

    delete neighbours;
  }

  LOG4CXX_INFO(logger, "Built region tree");
  LOG4CXX_DEBUG(logger, "Got " << regionLeaves->size() << " regions with the darkest shade");

  QImage output(input);

  for (RegionSet::iterator i = regionLeaves->begin(), e = regionLeaves->end(); i != e; ++i) {
    LOG4CXX_DEBUG(logger, "Setting up region walk for region " << *i);

    RegionWalker *walker = new RegionWalker(*i, delta);
    ResultSet *results = walker->findMSER();
    delete walker;

    for (ResultSet::iterator ri = results->begin(), re = results->end(); ri != re; ++ri) {
      paintRegion((*ri)->region, &output);
    }
  }

  LOG4CXX_INFO(logger, "Saving output");

  if (!output.save(outputFilename->c_str())) {
    LOG4CXX_ERROR(logger, "Couldn't save image!");
  }

  return 0;
}

void paintRegion(mser::Region *region, QImage *output) {
  int depth = output->depth();
  RegionSet *toPaint = new RegionSet();
  toPaint->insert(region);

  while (!toPaint->isEmpty()) {
    mser::Region *r = *(toPaint->begin());
    toPaint->remove(r);
    toPaint->unite(*(r->children));

    for (PixelVector::iterator pi = r->pixels->begin(), pe = r->pixels->end();
        pi != pe; ++pi) {

      if (depth > 8) {
        output->setPixel((*pi)->x, (*pi)->y, 0xffff0000);
      } else {
        output->setPixel((*pi)->x, (*pi)->y, 80);
      }
    }
  }
}

inline void mergeRegions(mser::Region *merge, mser::Region *into) {
  assert(merge && into);
  assert(merge->gray == into->gray);

  for (RegionSet::iterator i = merge->children->begin(), e = merge->children->end();
      i != e; ++i) {
    (*i)->parent = into;
    into->children->insert(*i);
  }

  for (PixelVector::iterator i = merge->pixels->begin(), e = merge->pixels->end();
      i != e; ++i) {
    (*i)->region = into;
    into->pixels->append(*i);
  }

  into->size += merge->size;
}

RegionSet* walkRegions(mser::Region *current, mser::Region *lower,
    mser::Region *upper, double lastqi, bool lastWasMin) {

  RegionSet *regionsFound = new RegionSet();
  mser::Region *lastRegion = current->parent;

  double qi;

  while (true) {
    qi = ((double) (upper->size - lower->size)) / ((double) current->size);
    LOG4CXX_TRACE(logger, qi);

    if (lastWasMin && (qi > (lastqi + 0.02))) {
      LOG4CXX_DEBUG(logger, "Found minimum!");
      regionsFound->insert(lastRegion);
      lastWasMin = false;
    }

    if ((qi + 0.02) < lastqi) {
      lastWasMin = true;
      lastRegion = current;
    }

    // Find branches taken further up, if upper & current are branching too.
    mser::Region *nextCurrent = lower;
    while (nextCurrent->parent != current) {
      nextCurrent = nextCurrent->parent;
    }
    current = nextCurrent;

    mser::Region *nextUpper = lower;
    while (nextUpper->parent != upper) {
      nextUpper = nextUpper->parent;
    }
    upper = nextUpper;

    // Reached a leaf or branching.
    if (lower->children->isEmpty() || lower->children->size() > 1) {
      for (RegionSet::iterator lci = lower->children->begin(), lce = lower->children->end();
          lci != lce; ++lci) {

        RegionSet *branchedFound = walkRegions(current, *lci, upper, lastqi, lastWasMin);
        regionsFound->unite(*branchedFound);
        delete branchedFound;
      }

      // Ugly control flow, I know.
      if (lower->children->isEmpty() && lastWasMin) {
        regionsFound->insert(lastRegion);
      }
      // Finished.
      break;
    } else {
      lower = *(lower->children->begin());
    }
  }

  return regionsFound;
}

RegionSet* findLowers(mser::Region *region, int depth) {
  RegionSet *lowers = new RegionSet();

  if (depth > 1) {
    for (RegionSet::iterator i = region->children->begin(), e = region->children->end();
        i != e; ++i) {
      RegionSet *childrenLowers = findLowers(*i, depth - 1);
      lowers->unite(*childrenLowers);
      delete childrenLowers;
    }
  } else {
    lowers->insert(region);
  }

  return lowers;
}

void dumpRegions(mser::Region *region, int indent, int depth) {
  for (int i = 0; i < indent; ++i) {
    cerr << " ";
  }

  cerr << region << ": size = " << region->size << " gray = " << (int) region->gray
      << " pixels = " << region->pixels->size() << endl;
  if (depth > 0) {
    for (RegionSet::iterator i = region->children->begin(), e = region->children->end();
        i != e; ++i) {
      dumpRegions(*i, indent + 1, depth - 1);
    }
  }
}
