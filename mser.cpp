#include "Pixel.h"
#include "QuadTree.h"

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QRgb>

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <unistd.h>

using namespace std;
using namespace mser;

static char *opts = "d:";

RegionSet* walkRegions(mser::Region *current, mser::Region *lower, mser::Region *upper);

void mergeRegions(mser::Region *merge, mser::Region *into);

int main(int argc, char *argv[]) {
  short delta = 5;

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

  // TODO(hermannloose): Let user choose which file to work on.
  QImage input("input.png");

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

  // Find smallest containing power of two, brute force because we're expecting
  // rather small pictures.
  int quadtreeSize = 1;
  while ((quadtreeSize < width) && (quadtreeSize < height)) {
    quadtreeSize *= 2;
  }

  cerr << "Using quadtree of size " << quadtreeSize << "." << endl;

  // TODO(hermannloose): Experiment with a sweet spot for the last two parameters.
  QuadTree *qtree = new QuadTree(new QRect(0, 0, quadtreeSize, quadtreeSize), 20, 4);

  QRect neighbourhood = QRect(0, 0, 3, 3);

  RegionSet *regionLeaves = new RegionSet();

  int pixelsWithNeighbours = 0;
  for (PixelVector::iterator i = sortedPixels->begin(), e = sortedPixels->end(); i != e; ++i) {
    Pixel *pixel = *i;

    // Range check for 8-neighbourhood.
    QPoint position = (*i)->position;
    neighbourhood.moveTo(position.x() - 1, position.y() - 1);

    PixelVector *neighbours = qtree->queryRange(&neighbourhood);

    if (neighbours->isEmpty()) {
      // Start a new region.
      mser::Region *region = new mser::Region();
      region->gray = pixel->gray;
      pixel->region = region;
      region->pixels->append(pixel);
      region->size = 1;

      regionLeaves->insert(region);
    } else {
      /*
      cerr << "(" << ++pixelsWithNeighbours << ") " << pixel << " connecting "
          << neighbours->size() << endl;
          */

      RegionSet *rootRegions = Pixel::getRootRegions(neighbours);

      // Speculative new region.
      mser::Region *region = new mser::Region();
      region->gray = pixel->gray;

      for (RegionSet::iterator ri = rootRegions->begin(), re = rootRegions->end();
          ri != re; ++ri) {

        if ((*ri)->gray > pixel->gray) {
          (*ri)->parent = region;
          region->children->insert(*ri);
          region->size += (*ri)->size;
        } else {
          assert((*ri)->gray == pixel->gray);
          // We've found a preexisting region for the same threshold, merge.
          mergeRegions(region, *ri);
          //delete region;
          region = *ri;
        }
      }

      pixel->region = region;
      region->pixels->append(pixel);
      region->size += 1;
    }

    qtree->insert(pixel);
  }

  cerr << "Built region tree." << endl;

  QImage output(input);

  cerr << "Examining " << regionLeaves->size() << " leaf regions." << endl;

  for (RegionSet::iterator i = regionLeaves->begin(), e = regionLeaves->end(); i != e; ++i) {
    if ((*i)->size > 1) {
      cerr << "size: " << (*i)->size << "; ";
    }
    mser::Region *current = *i;
    mser::Region *lower = *i;
    mser::Region *upper = *i;
    bool failedToSpan = false;

    for (int x = 0; x < delta; ++x) {
      if (current->parent) {
        current = current->parent;
      } else {
        //cerr << x;
        failedToSpan = true;
        break;
      }
    }

    for (int x = 0; x < delta * 2; ++x) {
      if (upper->parent) {
        upper = upper->parent;
      } else {
        //cerr << x;
        failedToSpan = true;
        break;
      }
    }

    if (failedToSpan) {
      continue;
    } else {
      //cerr << endl;
    }

    RegionSet *regionsFound = walkRegions(current, lower, upper);

    cerr << "Found " << regionsFound->size() << " regions for starting region "
        << lower << "." << endl;

    for (RegionSet::iterator fi = regionsFound->begin(), fe = regionsFound->end();
        fi != fe; ++fi) {

      for (PixelVector::iterator pi = (*fi)->pixels->begin(), pe = (*fi)->pixels->end();
          pi != pe; ++pi) {

        output.setPixel((*pi)->position, 0xffff0000);
      }
    }
  }

  cerr << "Saving output." << endl;

  // TODO(hermannloose): Let user choose filename, maybe append kernel size.
  if (!output.save("output.png")) {
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

  into->size += merge->size;
  *(into->pixels) << *(merge->pixels);
}

RegionSet* walkRegions(mser::Region *current, mser::Region *lower,
    mser::Region *upper) {

  RegionSet *regionsFound = new RegionSet();
  mser::Region *lastRegion;

  double minqi = 1000000;
  double qi = 1000000;

  while (upper != NULL) {
    qi = (double) (upper->size - lower->size) / (double) current->size;

    cerr << "qi: " << qi << endl;

    if (qi >= minqi) {
      regionsFound->insert(lastRegion);
    } else {
      minqi = qi;
      lastRegion = current;
    }

    current = current->parent;
    lower = lower->parent;
    upper = upper->parent;
  }

  if (qi == minqi) {
    regionsFound->insert(lastRegion);
  }

  return regionsFound;
}
