DESTDIR = ../../lib
TARGET = hidapi
TEMPLATE = lib
CONFIG += staticlib warn_off

include(../../build-config.prf)

INCLUDEPATH += .

HEADERS += hidapi.h

win32 {
    LIBS += -lsetupapi
    SOURCES += windows/hid.c
}

macx {
    QMAKE_LFLAGS += -F/System/Library/Frameworks
    # MacOS version using libusb and hidapi codes
    SOURCES += mac/hid.c

    LIBS += \
        -framework CoreFoundation \
        -framework IOKit \
}

unix:!macx {
    # Linux version using libusb and hidapi codes
    SOURCES += linux/hid-libusb.c
}
