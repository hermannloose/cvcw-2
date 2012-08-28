#include "Pixel.h"
#include "QuadTree.h"

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

  if (!input.isGrayscale()) {
    cerr << "Image isn't grayscale, aborting." << endl;
    exit(1);
  }

  int width = input.width();
  int height = input.height();

  cerr << "Got image [" << width << "x" << height <<"]." << endl;

  PixelVector *pixels = new PixelVector();
  pixels->reserve(width * height);

  cerr << "Building pixels." << endl;

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      pixels->append(new Pixel(QPoint(x, y), qGray(input.pixel(x, y))));
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

  // The root region of the whole region tree.
  mser::Region *rootRegion;

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

      rootRegion = region;
    } else {
      RegionSet *rootRegions = Pixel::getRootRegions(neighbours);

      // Speculative new region.
      mser::Region *region = new mser::Region();
      region->gray = pixel->gray;

      for (RegionSet::iterator ri = rootRegions->begin(), re = rootRegions->end();
          ri != re; ++ri) {

        if ((*ri)->gray < pixel->gray) {
          (*ri)->parent = region;
          region->children->insert(*ri);
        } else {
          // We've found a preexisting region for the same threshold, merge.
          for (RegionSet::iterator ci = region->children->begin(), ce = region->children->end();
              ci != ce; ++ci) {

            (*ci)->parent = (*ri);
            (*ri)->children->insert(*ci);
            region = *ri;
          }
        }
      }

      pixel->region = region;
      region->pixels->append(pixel);

      rootRegion = region;
    }

    qtree->insert(pixel);
  }

  cerr << "Built region tree." << endl;




  QImage output(input);

  /*
  QuadTree *qtree = new QuadTree(new QRect(0, 0, 1024, 1024), 3);
  qtree->insert(QPoint(1, 1));
  qtree->insert(QPoint(19, 27));
  qtree->insert(QPoint(3, 252));
  qtree->insert(QPoint(783, 19));
  qtree->insert(QPoint(3, 2));
  qtree->insert(QPoint(1, 0));
  qtree->insert(QPoint(7, 12));

  PointVector *inRange = qtree->queryRange(QRect(0, 0, 32, 32));
  for (PointVector::iterator i = inRange->begin(), e = inRange->end(); i != e; ++i) {
    cerr << "(" << i->x() << ", " << i->y() << ") is in range." << endl;
  }
  */

  cerr << "Saving output." << endl;

  // TODO(hermannloose): Let user choose filename, maybe append kernel size.
  if (!output.save("output.png")) {
    cerr << "Couldn't save image!" << endl;
  }

  return 0;
}
