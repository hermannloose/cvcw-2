CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Pixel.h QuadTree.h
SOURCES += mser.cpp Pixel.cpp QuadTree.cpp
