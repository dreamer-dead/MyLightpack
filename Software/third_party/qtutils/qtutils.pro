DESTDIR = ../../lib
TARGET = qtutils
TEMPLATE = lib
CONFIG += staticlib warn_off

include(../../build-config.prf)

INCLUDEPATH += ./include

HEADERS += include/QTUtils.hpp \
           include/ThreadedObject.hpp \
           include/RegisteredThread.hpp

SOURCES += src/RegisteredThread.cpp
