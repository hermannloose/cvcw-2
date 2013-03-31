CONFIG += qt debug
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

HEADERS += Path.h Pixel.h PixelImage.h RegionWalker.h
SOURCES += mser.cpp Path.cpp Pixel.cpp PixelImage.cpp RegionWalker.cpp
LIBS += -llog4cxx
