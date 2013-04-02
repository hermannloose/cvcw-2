CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Globals.h Path.h Pixel.h PixelImage.h Region.h RegionWalker.h
SOURCES += mser.cpp Path.cpp Pixel.cpp PixelImage.cpp Region.cpp RegionWalker.cpp
LIBS += -llog4cxx
