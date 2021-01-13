VERSION = 0.2.0alpha0

# Check Qt version.
requires(equals(QT_MAJOR_VERSION, 5))

QT += core gui multimedia multimediawidgets xml widgets
# Build in multimedia support is not available in Qt 6.0, but should be
# available in Qt 6.2, expected in 2021-09.

TARGET = beamerpresenter
TEMPLATE = app

# Include Poppler: requires that poppler-qt5 libraries are installed.
# Tested with poppler 20.12.1
DEFINES += INCLUDE_POPPLER

# Include MuPDF: requires that mupdf libraries are installed.
# Tested only with libmupdf 1.17.0
DEFINES += INCLUDE_MUPDF

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Set git version for more precise version info if possible.
exists(.git) {
    VERSION_STRING = "$${VERSION}-$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-list --count HEAD ).$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short HEAD )"
} else {
    VERSION_STRING = "$${VERSION}"
}

# Define the application version.
DEFINES += APP_VERSION=\\\"$${VERSION_STRING}\\\"

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
        src/gui/actionbutton.cpp \
        src/gui/clockwidget.cpp \
        src/gui/drawtoolbutton.cpp \
        src/gui/flexlayout.cpp \
        src/gui/keyinputlabel.cpp \
        src/gui/noteswidget.cpp \
        src/gui/settingswidget.cpp \
        src/gui/slidelabelwidget.cpp \
        src/gui/slidenumberwidget.cpp \
        src/gui/stackedwidget.cpp \
        src/gui/tabwidget.cpp \
        src/gui/timerwidget.cpp \
        src/gui/tocwidget.cpp \
        src/gui/toolselectorwidget.cpp \
        src/main.cpp \
        src/master.cpp \
        src/pdfmaster.cpp \
        src/preferences.cpp \
        src/rendering/externalrenderer.cpp \
        src/rendering/pdfdocument.cpp \
        src/rendering/pixcache.cpp \
        src/rendering/pixcachethread.cpp \
        src/rendering/pngpixmap.cpp \
        src/drawing/pathcontainer.cpp \
        src/drawing/basicgraphicspath.cpp \
        src/drawing/fullgraphicspath.cpp \
        src/slidescene.cpp \
        src/slideview.cpp

HEADERS += \
        src/drawing/drawtool.h \
        src/drawing/flexgraphicslineitem.h \
        src/drawing/tool.h \
        src/enumerates.h \
        src/gui/actionbutton.h \
        src/gui/clockwidget.h \
        src/gui/containerwidget.h \
        src/gui/drawtoolbutton.h \
        src/gui/flexlayout.h \
        src/gui/keyinputlabel.h \
        src/gui/noteswidget.h \
        src/gui/settingswidget.h \
        src/gui/slidelabelwidget.h \
        src/gui/slidenumberwidget.h \
        src/gui/stackedwidget.h \
        src/gui/tabwidget.h \
        src/gui/timerwidget.h \
        src/gui/tocwidget.h \
        src/gui/toolselectorwidget.h \
        src/master.h \
        src/names.h \
        src/pdfmaster.h \
        src/preferences.h \
        src/rendering/abstractrenderer.h \
        src/rendering/pdfdocument.h \
        src/rendering/externalrenderer.h \
        src/rendering/pixcache.h \
        src/rendering/pixcachethread.h \
        src/rendering/pngpixmap.h \
        src/drawing/pathcontainer.h \
        src/drawing/abstractgraphicspath.h \
        src/drawing/basicgraphicspath.h \
        src/drawing/fullgraphicspath.h \
        src/slidescene.h \
        src/slideview.h

contains(DEFINES, INCLUDE_POPPLER) {
    SOURCES += \
            src/rendering/popplerdocument.cpp
    HEADERS += \
            src/rendering/popplerrenderer.h \
            src/rendering/popplerdocument.h \
}

contains(DEFINES, INCLUDE_MUPDF) {
    SOURCES += \
            src/rendering/mupdfrenderer.cpp \
            src/rendering/mupdfdocument.cpp
    HEADERS += \
            src/rendering/mupdfrenderer.h \
            src/rendering/mupdfdocument.h
    DEFINES += FITZ_DEBUG_LOCKING
}

unix {
    LIBS += -L/usr/lib/
    contains(DEFINES, INCLUDE_POPPLER) {
        INCLUDEPATH += /usr/include/poppler/qt5
        LIBS += -lpoppler-qt5
    }
    contains(DEFINES, INCLUDE_MUPDF) {
        INCLUDEPATH += /usr/include/mupdf
        LIBS += -lmupdf -lmupdf-third -lm -lfreetype -lz -lharfbuzz -ljpeg -ljbig2dec -lopenjp2
    }
}
macx {
    ## Please configure this according to your poppler and/or MuPDF installation.
    ## Installation on Mac is untested.
    #contains(DEFINES, INCLUDE_POPPLER) {
    #    INCLUDEPATH += /usr/local/opt/poppler/include
    #    LIBS += -L/usr/local/opt/poppler/lib/ -lpoppler-qt5
    #}
    #contains(DEFINES, INCLUDE_MUPDF) {
    #    INCLUDEPATH += /usr/local/opt/mupdf/include
    #    LIBS += -L/usr/local/opt/mupdf/lib/ -lmupdf -lmupdf-third -lm -lfreetype -lz -lharfbuzz -ljpeg -ljbig2dec -lopenjp2
    #}
}
win32 {
    ## Please configure this according to your poppler and/or MuPDF installation.
    #contains(DEFINES, INCLUDE_POPPLER) {
    #    # The configuration will probably have the following form:
    #    INCLUDEPATH += C:\...\poppler-0.??.?-win??
    #    LIBS += -LC:\...\poppler-0.??.?-win?? -lpoppler-qt5
    #}
    #contains(DEFINES, INCLUDE_MUPDF) {
    #    INCLUDEPATH += ...
    #    LIBS += ...
    #}
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
