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
#include <sstream>
#include <stdlib.h>
#include <string>
#include <unistd.h>

using namespace mser;

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

static char *opts = "d:";

RegionSet* placePixels(PixelVector *sortedPixels, int width, int height);

void paintRegion(mser::Region *region, QImage *output);

void paintRegionTree(mser::Region *root, QImage *original);

void mergeRegions(mser::Region *merge, mser::Region *into);

LoggerPtr logger(Logger::getLogger("mser"));
LoggerPtr MinimumFinder::logger(Logger::getLogger("mser.MinimumFinder"));
LoggerPtr Path::logger(Logger::getLogger("mser.Path"));
LoggerPtr mser::Region::logger(Logger::getLogger("mser.Region"));
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

  LOG4CXX_INFO(logger, "Got image [" << width << "x" << height << "] = "
      << width * height << " pixels");
  LOG4CXX_INFO(logger, "Using delta of " << delta);

  PixelVector *pixels = new PixelVector();
  pixels->reserve(width * height);

  LOG4CXX_DEBUG(logger, "Building pixels");

  for (int x = 0; x < width; ++x) {
    for (int y = 0; y < height; ++y) {
      pixels->append(new Pixel(x, y, qGray(input.pixel(x, y))));
    }
  }

  LOG4CXX_INFO(logger, "Sorting pixels");

  PixelVector *sortedPixels = Pixel::binSort(pixels);
  delete pixels;
  reverse(sortedPixels->begin(), sortedPixels->end());

  RegionSet *regionLeaves = placePixels(sortedPixels, width, height);

  QImage output = input.convertToFormat(QImage::Format_RGB32);

  for (RegionSet::iterator i = regionLeaves->begin(), e = regionLeaves->end(); i != e; ++i) {
    LOG4CXX_DEBUG(logger, "Setting up region walk for region " << *i);

    RegionWalker *walker = new RegionWalker(*i, delta);

    LOG4CXX_INFO(logger, "Finding MSERs");
    ResultSet *results = walker->findMSER();
    LOG4CXX_INFO(logger, "Found MSERs");

    for (ResultSet::iterator ri = results->begin(), re = results->end(); ri != re; ++ri) {
      LOG4CXX_DEBUG(logger, "Painting MSER " << *ri << " of size " << (*ri)->region->size);
      paintRegion((*ri)->region, &output);
    }

    delete walker;
    delete results;
  }

  LOG4CXX_INFO(logger, "Painting region tree");

  paintRegionTree(*(regionLeaves->begin()), &input);

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
    toPaint->unite(*(r->exposeChildren()));

    for (PixelVector::iterator pi = r->pixelsBegin(), pe = r->pixelsEnd();
        pi != pe; ++pi) {
      output->setPixel((*pi)->x, (*pi)->y, 0xffff0000);
    }
  }
}

inline void mergeRegions(mser::Region *merge, mser::Region *into) {
  assert(merge && into);
  assert(merge->gray == into->gray);

  merge->mergeInto(into);
}

RegionSet* placePixels(PixelVector *sortedPixels, int width, int height) {
  LOG4CXX_INFO(logger, "Placing pixels");

  PixelImage *pimage = new PixelImage(width, height);
  RegionSet *regionLeaves = new RegionSet();

  RegionSet *toMerge = new RegionSet();
  RegionSet *toGroup = new RegionSet();

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

      LOG4CXX_TRACE(logger, rootRegions->size() << " root regions for neighbours of "
          << pixel->x << "x" << pixel->y);

      toMerge->clear();
      toGroup->clear();

      for (RegionSet::iterator ri = rootRegions->begin(), re = rootRegions->end();
          ri != re; ++ri) {
        if ((*ri)->gray == pixel->gray) {
          toMerge->insert(*ri);
        } else {
          assert((*ri)->gray > pixel->gray);
          toGroup->insert(*ri);
        }
      }

      delete rootRegions;

      mser::Region *region;
      if (toMerge->isEmpty()) {
        region = new mser::Region();
        region->gray = pixel->gray;
      } else {
        for (RegionSet::iterator ri = toMerge->begin(); ri != toMerge->end(); ++ri) {
          if (ri == toMerge->begin()) {
            region = *ri;
          } else {
            mser::Region *toDelete = *ri;
            toDelete->mergeInto(region);
            LOG4CXX_TRACE(logger, "Erasing merged region " << *ri);
            ri = toMerge->erase(ri);
            regionLeaves->remove(toDelete);
            --ri;
            delete toDelete;
          }
        }
      }

      pixel->region = region;
      region->pixels->append(pixel);
      region->size += 1;
      assert(region->size >= 0);

      for (RegionSet::iterator ri = toGroup->begin(), re = toGroup->end(); ri != re; ++ri) {
        (*ri)->groupUnder(region);
        regionLeaves->remove(*ri);
      }

      regionLeaves->insert(region);
    }

    pimage->insert(pixel);

    delete neighbours;
  }

  LOG4CXX_INFO(logger, "Built region tree");
  LOG4CXX_DEBUG(logger, "Got " << regionLeaves->size() << " regions with the darkest shade");

  assert(regionLeaves->size() == 1);

  return regionLeaves;
}

void paintRegionTree(mser::Region *root, QImage *original) {
  RegionSet *toPaint[256];

  for (int i = 0; i < 256; ++i) {
    toPaint[i] = new RegionSet();
  }

  RegionVector *toProcess = new RegionVector();
  toProcess->append(root);

  QMutableVectorIterator<mser::Region*> i(*toProcess);
  while (i.hasNext()) {
    mser::Region *current = i.next();
    i.remove();
    toPaint[current->gray]->insert(current);
    i.toBack();
    for (RegionSet::iterator ri = current->exposeChildren()->begin(),
        re = current->exposeChildren()->end(); ri != re; ++ri) {
      i.insert(*ri);
    }
    i.toFront();
  }

  srand(0);

  for (int i = 0; i < 256; ++i) {
    if (toPaint[i]->isEmpty()) {
      continue;
    }

    QImage output = original->convertToFormat(QImage::Format_RGB32);

    for (RegionSet::iterator ri = toPaint[i]->begin(), re = toPaint[i]->end(); ri != re; ++ri) {
      unsigned long color = 0xff000000;
      color |= (rand() % 255) << 16;
      color |= (rand() % 255) << 8;
      color |= (rand() % 255);

      RegionVector *process = new RegionVector();
      process->append(*ri);

      QMutableVectorIterator<mser::Region*> processi(*process);
      while (processi.hasNext()) {
        mser::Region *current = processi.next();
        processi.remove();

        processi.toBack();
        for (RegionSet::iterator rri = current->exposeChildren()->begin(),
            rre = current->exposeChildren()->end(); rri != rre; ++rri) {
          processi.insert(*rri);
        }
        processi.toFront();

        for (PixelVector::iterator pi = current->pixelsBegin(), pe = current->pixelsEnd();
            pi != pe; ++pi) {
          output.setPixel((*pi)->x, (*pi)->y, color);
        }
      }
    }

    ostringstream regionFileName;
    regionFileName << "regions" << i << ".png";
    output.save(regionFileName.str().c_str());
  }
}
