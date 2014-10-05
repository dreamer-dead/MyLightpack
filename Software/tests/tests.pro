#-------------------------------------------------
#
# Project created by QtCreator 2011-09-09T12:09:24
#
#-------------------------------------------------

QT         += widgets network testlib

TARGET      = LightpackTests
DESTDIR     = bin

CONFIG     += console
CONFIG     -= app_bundle

CONFIG(gcc):QMAKE_CXXFLAGS += -std=c++11

# QMake and GCC produce a lot of stuff
OBJECTS_DIR = tests_stuff
MOC_DIR     = tests_stuff
UI_DIR      = tests_stuff
RCC_DIR     = tests_stuff

# Grabber types configuration
include(../grab/configure-grabbers.prf)
DEFINES += $${SUPPORTED_GRABBERS}

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L../lib -lprismatik-math -lgrab

win32 {
    CONFIG(msvc):DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    LIBS += -ladvapi32
}

INCLUDEPATH += . \
               ../prismatic \
               ../prismatic/settings \
               ../hooks \
               ../grab/include \
               ../math/include \
               ..


HEADERS += \
    ../common/defs.h \
    ../prismatic/enums.hpp \
    ../prismatic/ApiServerSetColorTask.hpp \
    ../prismatic/ApiServer.hpp \
    ../prismatic/debug.h \
    ../prismatic/settings/Settings.hpp \
    ../prismatic/settings/SettingsSignals.hpp \
    ../prismatic/Plugin.hpp \
    ../prismatic/LightpackPluginInterface.hpp \
    ../prismatic/LightpackCommandLineParser.hpp \
    ../grab/include/calculations.hpp \
    ../grab/include/GrabberContext.hpp \
    ../math/include/PrismatikMath.hpp \
    SettingsWindowMockup.hpp \
    GrabCalculationTest.hpp \
    LightpackApiTest.hpp \
    lightpackmathtest.hpp \
    AppVersionTest.hpp \
    ../prismatic/UpdatesProcessor.hpp \
    SettingsTest.hpp \
    SettingsSourceMockupTest.hpp \
    SettingsSourceMockup.hpp \
    LightpackCommandLineParserTest.hpp \
    GrabTests.hpp

SOURCES += \
    ../prismatic/ApiServerSetColorTask.cpp \
    ../prismatic/ApiServer.cpp \
    ../prismatic/settings/Settings.cpp \
    ../prismatic/settings/DeviceTypesInfo.cpp \
    ../prismatic/settings/ConfigurationProfile.cpp \
    ../prismatic/settings/SettingsProfiles.cpp \
    ../prismatic/settings/SettingsSignals.cpp \
    ../prismatic/Plugin.cpp \
    ../prismatic/LightpackPluginInterface.cpp \
    ../prismatic/LightpackCommandLineParser.cpp \
    LightpackApiTest.cpp \
    SettingsWindowMockup.cpp \
    GrabCalculationTest.cpp \
    lightpackmathtest.cpp \
    TestsMain.cpp \
    AppVersionTest.cpp \
    ../prismatic/UpdatesProcessor.cpp \
    SettingsTest.cpp \
    SettingsSourceMockup.cpp \
    SettingsSourceMockupTest.cpp \
    LightpackCommandLineParserTest.cpp \
    GrabTests.cpp

win32{
    HEADERS += \
        HooksTest.h \
        ../hooks/ProxyFuncJmp.hpp \
        ../hooks/ProxyFunc.hpp \
        ../hooks/hooksutils.h \
        ../hooks/ProxyFuncVFTable.hpp \
        ../hooks/Logger.hpp

    SOURCES += \
        HooksTest.cpp \
        ../hooks/ProxyFuncJmp.cpp \
        ../hooks/hooksutils.cpp \
        ../hooks/ProxyFuncVFTable.cpp \
        ../hooks/Logger.cpp
}
