#pragma once

#include <QPoint>
#include <QVector>

class Pixel;

typedef QVector<Pixel> PixelVector;

class Pixel {

  public:
    Pixel();
    Pixel(QPoint position, int gray);

    QPoint getPosition();
    int getGray();

    static PixelVector* binSort(PixelVector *original);

  private:
    QPoint position;
    int gray;
};
