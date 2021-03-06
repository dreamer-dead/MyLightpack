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

include(../build-config.prf)

CONFIG(gcc):QMAKE_CXXFLAGS += -std=c++11
CONFIG(clang) {
    QMAKE_CXXFLAGS += -std=c++11 -stdlib=libc++
    LIBS += -stdlib=libc++
}

# QMake and GCC produce a lot of stuff
OBJECTS_DIR = tests_stuff
MOC_DIR     = tests_stuff
UI_DIR      = tests_stuff
RCC_DIR     = tests_stuff

# Grabber types configuration
include(../grab/configure-grabbers.prf)
DEFINES += $${SUPPORTED_GRABBERS}

CONFIG(gcc) {
    QMAKE_CXXFLAGS += -isystem ../third_party/gtest/include
    QMAKE_CXXFLAGS += -g -Wall -Wextra -pthread -Wno-missing-field-initializers
}

DEFINES += SRCDIR=\\\"$$PWD/\\\"

LIBS += -L../lib -lprismatik-math -lgrab -lqtutils

win32 {
    CONFIG(msvc):DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_DEPRECATE
    LIBS += -ladvapi32
}

INCLUDEPATH += . \
               .. \
               ../third_party/gtest/include \
               ../third_party/gtest \
               ../prismatic \
               ../prismatic/settings \
               ../hooks \
               ../grab/include \
               ../math/include \

HEADERS += \
    # Use undocumented $$file function to list files with a wildcard.
    # List needed Google Test files
    $$files(../third_party/gtest/include/gtest/*.h) \
    $$files(../third_party/gtest/include/gtest/internal/*.h) \
    $$files(../third_party/gtest/src/*.h) \
    ../common/defs.h \
    ../grab/include/calculations.hpp \
    ../grab/include/GrabberContext.hpp \
    ../math/include/PrismatikMath.hpp \
    ../prismatic/ApiServer.hpp \
    ../prismatic/ApiServerSetColorTask.hpp \
    ../prismatic/enums.hpp \
    ../prismatic/LightpackCommandLineParser.hpp \
    ../prismatic/LightpackPluginInterface.hpp \
    ../prismatic/Plugin.hpp \
    ../prismatic/PluginsManager.hpp \
    ../prismatic/settings/Settings.hpp \
    ../prismatic/settings/SettingsSignals.hpp \
    ../prismatic/UpdatesProcessor.hpp \
    mocks/SettingsSourceMockup.hpp \
    mocks/SettingsWindowMockup.hpp \
    mocks/SignalAndSlotObject.hpp \
    mocks/ProcessWaiter.hpp

SOURCES += \
    ../third_party/gtest/src/gtest-all.cc \
    ../prismatic/ApiServer.cpp \
    ../prismatic/ApiServerSetColorTask.cpp \
    ../prismatic/LightpackCommandLineParser.cpp \
    ../prismatic/LightpackPluginInterface.cpp \
    ../prismatic/Plugin.cpp \
    ../prismatic/PluginsManager.cpp \
    ../prismatic/settings/ConfigurationProfile.cpp \
    ../prismatic/settings/DeviceTypesInfo.cpp \
    ../prismatic/settings/Settings.cpp \
    ../prismatic/settings/SettingsProfiles.cpp \
    ../prismatic/settings/SettingsSignals.cpp \
    ../prismatic/UpdatesProcessor.cpp \
    AppVersionTest.cpp \
    GrabCalculationTest.cpp \
    GrabTests.cpp \
    LightpackApiTest.cpp \
    LightpackCommandLineParserTest.cpp \
    lightpackmathtest.cpp \
    mocks/SettingsSourceMockup.cpp \
    mocks/SettingsWindowMockup.cpp \
    SettingsSourceMockupTest.cpp \
    SettingsTest.cpp \
    TestsMain.cpp \
    ConnectorTests.cpp \
    QtUtilsTests.cpp \
    mocks/SignalAndSlotObject.cpp \
    CommandSetColorParsingTests.cpp \
    PluginTest.cpp \
    PluginsManagerTest.cpp \
    mocks/ProcessWaiter.cpp

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

OTHER_FILES += \
    data/plugins/info/info.ini \
    data/plugins/simple/simple.ini \
    data/plugins/kill/kill.ini
