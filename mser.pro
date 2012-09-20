CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Pixel.h PixelImage.h
SOURCES += mser.cpp Pixel.cpp PixelImage.cpp
