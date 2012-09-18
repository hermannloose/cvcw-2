CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Pixel.h PixelImage.h QuadTree.h
SOURCES += mser.cpp Pixel.cpp PixelImage.cpp QuadTree.cpp
