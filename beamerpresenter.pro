VERSION = 0.2.0alpha0


###########################################################
###   DEFINE PDF ENGINE, CUSTOM CONFIGURATION
###########################################################

# BeamerPresenter supports the PDF engines poppler and MuPDF.
# You can here select which PDF engine(s) to include in you installation.
# At least one PDF engine must be included.
# Library paths and linking arguments for the PDF engines need to be defined
# below in section "CONFIGURE LIBRARIES AND LINKING - OS DEPENDENT"

# Include Poppler: requires that poppler-qt5 libraries are installed.
# Tested with poppler 21.02.0, versions below 0.70 are not supported and will
# most probably never be supported.
DEFINES += INCLUDE_POPPLER

# Include MuPDF: requires that mupdf libraries are installed.
# Tested only with libmupdf 1.18.0
DEFINES += INCLUDE_MUPDF

# Define a path where the icon will be placed (don't forget the trailing /).
unix: ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"
#win32: ICON_PATH = "C:\..."
#macx: ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"

# Disable debugging message if debugging mode is disabled.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT


###########################################################
###   CONFIGURE QT + PREPROCESSOR OPTIONS
###########################################################

# Check Qt version.
# Build in multimedia support is not available in Qt 6.0, but should be
# available in Qt 6.2, expected in 2021-09.
requires(equals(QT_MAJOR_VERSION, 5))

# Check whether a PDF engine was defined.
requires(contains(DEFINES, INCLUDE_POPPLER) | contains(DEFINES, INCLUDE_MUPDF))

CONFIG += c++20 qt
QT += core gui multimedia multimediawidgets xml widgets

TARGET = beamerpresenter
TEMPLATE = app

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

# Define the application version string.
DEFINES += APP_VERSION=\\\"$${VERSION_STRING}\\\"

# Define the icon path string.
DEFINES += ICON_PATH=\\\"$${ICON_PATH}\\\"
unix {
    # Enable better debugging.
    CONFIG(debug, debug|release):QMAKE_LFLAGS += -rdynamic
}


###########################################################
###   DEFINE SOURCES
###########################################################

SOURCES += \
        src/drawing/pixmapgraphicsitem.cpp \
        src/gui/actionbutton.cpp \
        src/gui/clockwidget.cpp \
        src/gui/flexlayout.cpp \
        src/gui/keyinputlabel.cpp \
        src/gui/noteswidget.cpp \
        src/gui/settingswidget.cpp \
        src/gui/slidelabelwidget.cpp \
        src/gui/slidenumberwidget.cpp \
        src/gui/stackedwidget.cpp \
        src/gui/tabwidget.cpp \
        src/gui/thumbnailbutton.cpp \
        src/gui/thumbnailthread.cpp \
        src/gui/thumbnailwidget.cpp \
        src/gui/timerwidget.cpp \
        src/gui/tocbutton.cpp \
        src/gui/tocwidget.cpp \
        src/gui/toolbutton.cpp \
        src/gui/tooldialog.cpp \
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
        src/drawing/pixmapgraphicsitem.h \
        src/drawing/pointingtool.h \
        src/drawing/texttool.h \
        src/drawing/tool.h \
        src/enumerates.h \
        src/gui/actionbutton.h \
        src/gui/clockwidget.h \
        src/gui/containerwidget.h \
        src/gui/flexlayout.h \
        src/gui/keyinputlabel.h \
        src/gui/noteswidget.h \
        src/gui/settingswidget.h \
        src/gui/slidelabelwidget.h \
        src/gui/slidenumberwidget.h \
        src/gui/stackedwidget.h \
        src/gui/tabwidget.h \
        src/gui/thumbnailbutton.h \
        src/gui/thumbnailthread.h \
        src/gui/thumbnailwidget.h \
        src/gui/timerwidget.h \
        src/gui/tocbutton.h \
        src/gui/tocwidget.h \
        src/gui/toolbutton.h \
        src/gui/tooldialog.h \
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
    CONFIG(debug, debug|release):DEFINES += FITZ_DEBUG_LOCKING
}


###########################################################
###   CONFIGURE LIBRARIES AND LINKING - OS DEPENDENT
###########################################################

unix {
    LIBS += -L/usr/lib/
    contains(DEFINES, INCLUDE_POPPLER) {
        INCLUDEPATH += /usr/include/poppler/qt5
        LIBS += -lpoppler-qt5
    }
    contains(DEFINES, INCLUDE_MUPDF) {
        INCLUDEPATH += /usr/include/mupdf
        LIBS += -lmupdf -lmupdf-third -lm -lfreetype -lz -lharfbuzz -ljpeg -ljbig2dec -lopenjp2 -lgumbo
        # In my installation of MuPDF, the gumbo library is needed since MuPDF 1.18.0.
        # All other libraries were also required in MuPDF 1.17.0.
    }
}

macx {
    ## Please configure this according to your poppler and/or MuPDF installation.
    ## Installation on Mac is untested. The predefined configuration here is just a guess.
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
    #    INCLUDEPATH += C:\...\poppler-...-win??
    #    LIBS += -LC:\...\poppler-...-win?? -lpoppler-qt5
    #}
    #contains(DEFINES, INCLUDE_MUPDF) {
    #    INCLUDEPATH += ...
    #    LIBS += ...
    #}
}


###########################################################
###   CONFIGURE INSTALLATION - OS DEPENDENT
###########################################################

unix {
    # Commands needed for make install
    # Include man pages and default configuration in make install.

    man1.path = /usr/share/man/man1/
    man1.CONFIG = no_check_exist no_build
    man1.extra = gzip -kf9 doc/beamerpresenter.1; gzip -kf9 doc/beamerpresenter-ui.1
    man1.files = doc/beamerpresenter.1.gz doc/beamerpresenter-ui.1.gz

    man5.path = /usr/share/man/man5/
    man5.CONFIG = no_check_exist no_build
    man5.extra = gzip -kf9 doc/beamerpresenter.conf.5
    man5.files = doc/beamerpresenter.conf.5.gz

    configuration.path = /etc/xdg/$${TARGET}/
    configuration.CONFIG = no_build
    configuration.files = config/beamerpresenter.conf config/gui.json

    icon.path = $${ICON_PATH}
    icon.CONFIG = no_build
    icon.files = share/icons/beamerpresenter.svg

    doc.path = /usr/share/doc/beamerpresenter/
    doc.CONFIG = no_build
    doc.files = share/doc/README.html

    desktop.path = /usr/share/applications/
    desktop.CONFIG = no_build
    desktop.files = share/applications/beamerpresenter.desktop

    target.path = /usr/bin/

    INSTALLS += man1 man5 configuration icon doc desktop target
}
