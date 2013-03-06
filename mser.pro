CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Pixel.h PixelImage.h RegionWalker.h
SOURCES += mser.cpp Pixel.cpp PixelImage.cpp RegionWalker.cpp
LIBS += -llog4cxx
