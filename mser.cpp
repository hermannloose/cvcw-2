#include "Pixel.h"
#include "PixelImage.h"

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

using namespace std;
using namespace mser;

static char *opts = "d:";

RegionSet* walkRegions(mser::Region *current, mser::Region *lower, mser::Region *upper,
    double minqi, bool lastWasMin);

void mergeRegions(mser::Region *merge, mser::Region *into);

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

  // TODO(hermannloose): Let user choose which file to work on.
  QImage input(inputFilename->c_str());

  /*
  if (!input.isGrayscale()) {
    cerr << "Image isn't grayscale, aborting." << endl;
    exit(1);
  }
  */

  int width = input.width();
  int height = input.height();

  cerr << "Got image [" << width << "x" << height <<"]." << endl;
  cerr << "Using delta of " << delta << "." << endl;

  PixelVector *pixels = new PixelVector();
  pixels->reserve(width * height);

  cerr << "Building pixels." << endl;

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      pixels->append(new Pixel(QPoint(x, y), qGray(input.pixel(x, y))));
      //cerr << "[" << x << ", " << y << "] = " << qGray(input.pixel(x, y)) << endl;
    }
  }

  cerr << "Sorting pixels." << endl;

  PixelVector *sortedPixels = Pixel::binSort(pixels);
  delete pixels;
  reverse(sortedPixels->begin(), sortedPixels->end());
  int darkest = sortedPixels->at(0)->gray;

  cerr << "Placing pixels." << endl;

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
          (*ri)->parent = region;
          region->children->insert(*ri);
          region->size += (*ri)->size;
        } else {
          assert((*ri)->gray == pixel->gray);
          // We've found a preexisting region for the same threshold, merge.
          mergeRegions(region, *ri);

          if (pixel->gray == darkest) {
            regionLeaves->remove(region);
          }
          delete region;

          region = *ri;
        }
      }

      pixel->region = region;
      region->pixels->append(pixel);
      region->size += 1;

      delete rootRegions;
    }

    pimage->insert(pixel);

    if (pixel->gray == darkest) {
      regionLeaves->insert(pixel->region);
    }

    delete neighbours;
  }

  cerr << "Built region tree." << endl;
  cerr << "Got " << regionLeaves->size() << " regions with the darkest shade." << endl;

  QImage output(input);

  for (RegionSet::iterator i = regionLeaves->begin(), e = regionLeaves->end(); i != e; ++i) {
    cerr << "Setting up region walk for region " << *i << "." << endl;

    mser::Region *current = *i;
    mser::Region *lower = *i;
    mser::Region *upper = *i;
    bool failedToSpan = false;

    for (int x = 0; x < delta; ++x) {
      if (current->parent) {
        current = current->parent;
      } else {
        failedToSpan = true;
        break;
      }
    }

    for (int x = 0; x < delta * 2; ++x) {
      if (upper->parent) {
        upper = upper->parent;
      } else {
        failedToSpan = true;
        break;
      }
    }

    if (failedToSpan) {
      cerr << "Failed to span." << endl;
      continue;
    } else {
      //cerr << endl;
    }

    cerr << "Walking regions." << endl;

    // FIXME(hermannloose): Last two arguments are bogus.
    RegionSet *regionsFound = walkRegions(current, lower, upper, 1, false);

    cerr << "Found " << regionsFound->size() << " regions for starting region "
        << lower << "." << endl;

    while (!regionsFound->isEmpty()) {
      RegionSet::iterator first = regionsFound->begin();
      regionsFound->remove(*first);

      for (RegionSet::iterator fi = (*first)->children->begin(), fe = (*first)->children->end();
          fi != fe; ++fi) {

        regionsFound->insert(*fi);
      }

      for (PixelVector::iterator pi = (*first)->pixels->begin(), pe = (*first)->pixels->end();
          pi != pe; ++pi) {

        output.setPixel((*pi)->x, (*pi)->y, 0xffff0000);
      }
    }

    delete regionsFound;
  }

  cerr << "Saving output." << endl;

  if (!output.save(outputFilename->c_str())) {
    cerr << "Couldn't save image!" << endl;
  }

  return 0;
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

    if (lastWasMin && (qi > lastqi)) {
      cerr << "Found minimum!" << endl;
      regionsFound->insert(lastRegion);
      lastWasMin = false;
    }

    if (qi < lastqi) {
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
        cerr << "Leaf reached." << endl;
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
