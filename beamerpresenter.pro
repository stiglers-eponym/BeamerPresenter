VERSION = 0.2.0

# Check Qt version.
requires(greaterThan(QT_MAJOR_VERSION, 4))
equals(QT_MAJOR_VERSION, 5) {
    requires(greaterThan(QT_MINOR_VERSION, 5))
    lessThan(QT_MINOR_VERSION, 9):message("Using Qt version < 5.9 is untested!.")
}

QT += core gui multimedia multimediawidgets xml widgets

TARGET = beamerpresenter
TEMPLATE = app

# Include MuPDF: requires that mupdf libraries are installed.
DEFINES += INCLUDE_MUPDF

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Define the application version.
#DEFINES += APP_VERSION=\\\"$${VERSION}\\\"

# Set git version for more precise version info.
DEFINES += APP_VERSION="\\\"$${VERSION}-$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-list --count HEAD ).$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD )\\\""

# Define a path where the icon will be placed (don't forget the trailing /).
ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"
DEFINES += ICON_PATH=\\\"$${ICON_PATH}\\\"

CONFIG += c++20 qt
unix {
    # Enable better debugging.
    CONFIG(debug, debug|release):QMAKE_LFLAGS += -rdynamic
}

# Disable debugging message if debugging mode is disabled.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Enable specific debugging messages
CONFIG(debug, debug|release) {
    DEFINES += DEBUG_READ_CONFIGS
    DEFINES += DEBUG_CACHE
    DEFINES += DEBUG_RENDERING
    DEFINES += DEBUG_DRAWING
    DEFINES += DEBUG_KEY_ACTIONS
    DEFINES += DEBUG_TOOL_ACTIONS
    DEFINES += DEBUG_SLIDE_TRANSITIONS
    DEFINES += DEBUG_MULTIMEDIA
}

SOURCES += \
        src/drawhistorystep.cpp \
        src/externalrenderer.cpp \
        src/main.cpp \
        src/pathcontainer.cpp \
        src/pdfmaster.cpp \
        src/pixcache.cpp \
        src/pixcachethread.cpp \
        src/pngpixmap.cpp \
        src/popplerrenderer.cpp \
        src/preferences.cpp \
        src/slidescene.cpp \
        src/slideview.cpp

HEADERS += \
        src/abstractrenderer.h \
        src/drawhistorystep.h \
        src/enumerates.h \
        src/externalrenderer.h \
        src/pathcontainer.h \
        src/pdfmaster.h \
        src/pixcache.h \
        src/pixcachethread.h \
        src/pngpixmap.h \
        src/popplerrenderer.h \
        src/preferences.h \
        src/slidescene.h \
        src/slideview.h

contains(DEFINES, INCLUDE_MUPDF) {
    SOURCES += src/mupdfrenderer.cpp
    HEADERS += src/mupdfrenderer.h
}

unix {
    INCLUDEPATH += /usr/include/poppler/qt5
    LIBS += -L /usr/lib/ -lpoppler-qt5
}
macx {
    ## Please configure this according to your poppler installation.
    ## Installation on Mac is untested.
    #INCLUDEPATH += ...
    #LIBS += ...
}
win32 {
    ## Please configure this according to your poppler installation.
    ## The configuration will probably have the following form:
    #INCLUDEPATH += C:\...\poppler-0.??.?-win??
    #LIBS += -LC:\...\poppler-0.??.?-win?? -lpoppler-qt5
}

unix {
    # Commands needed for make install
    # Include man pages and default configuration in make install.

    man1.path = /usr/share/man/man1/
    man1.CONFIG = no_check_exist no_build
    man1.extra = gzip -9 man/$${TARGET}.1 || true
    man1.files = man/$${TARGET}.1.gz

    man5.path = /usr/share/man/man5/
    man5.CONFIG = no_check_exist no_build
    man5.extra = gzip -9 man/$${TARGET}.conf.5 || true
    man5.files = man/$${TARGET}.conf.5.gz

    configuration.path = /etc/$${TARGET}/
    configuration.CONFIG = no_build
    configuration.files = config/$${TARGET}.conf

    icon.path = $${ICON_PATH}
    icon.CONFIG = no_build
    icon.files = src/icons/beamerpresenter.svg

    target.path = /usr/bin/

    INSTALLS += man1 man5 configuration icon target
}
