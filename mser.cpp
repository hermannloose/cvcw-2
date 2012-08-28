#include "Pixel.h"
#include "QuadTree.h"

#include <QImage>
#include <QPoint>
#include <QRgb>

#include <assert.h>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

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

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      pixels->append(Pixel(QPoint(x, y), qGray(input.pixel(x, y))));
    }
  }

  PixelVector *sortedPixels = Pixel::binSort(pixels);

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

  // TODO(hermannloose): Let user choose filename, maybe append kernel size.
  if (!output.save("output.png")) {
    cerr << "Couldn't save image!" << endl;
  }

  return 0;
}
