#include "PixelImage.h"

namespace mser {

  PixelImage::PixelImage(int width, int height) :
      width(width),
      height(height) {

    columns = new ColumnVector();
    columns->reserve(width);

    for (int i = 0; i < width; ++i) {
      PixelVector *column = new PixelVector();
      column->reserve(height);
      column->fill(NULL, height);

      columns->append(column);
    }
  }

  void PixelImage::insert(Pixel *pixel) {
    PixelVector *column = columns->at(pixel->x);
    column->replace(pixel->y, pixel);
  }

  PixelVector* PixelImage::neighbours(Pixel *pixel) {
    PixelVector *neighbours = new PixelVector();
    neighbours->reserve(8);


    Pixel *neighbour = NULL;

    if (pixel->x > 0) {
      PixelVector *leftColumn = columns->at(pixel->x - 1);

      if (pixel->y > 0) {
        neighbour = leftColumn->at(pixel->y - 1);
        if (neighbour != NULL) {
          neighbours->append(neighbour);
        }
        neighbour = NULL;
      }

      neighbour = leftColumn->at(pixel->y);
      if (neighbour != NULL) {
        neighbours->append(neighbour);
      }
      neighbour = NULL;

      if (pixel->y < height - 1) {
        neighbour = leftColumn->at(pixel->y + 1);
        if (neighbour != NULL) {
          neighbours->append(neighbour);
        }
        neighbour = NULL;
      }
    }

    if (pixel->x < width - 1) {
      PixelVector *rightColumn = columns->at(pixel->x + 1);

      if (pixel->y > 0) {
        neighbour = rightColumn->at(pixel->y - 1);
        if (neighbour != NULL) {
          neighbours->append(neighbour);
        }
        neighbour = NULL;
      }

      neighbour = rightColumn->at(pixel->y);
      if (neighbour != NULL) {
        neighbours->append(neighbour);
      }
      neighbour = NULL;

      if (pixel->y < height - 1) {
        neighbour = rightColumn->at(pixel->y + 1);
        if (neighbour != NULL) {
          neighbours->append(neighbour);
        }
        neighbour = NULL;
      }
    }

    return neighbours;
  }

}
