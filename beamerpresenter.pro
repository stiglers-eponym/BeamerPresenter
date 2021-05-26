VERSION = 0.2.0

###########################################################
###   DEFINE PDF ENGINE, CUSTOM CONFIGURATION
###########################################################

# BeamerPresenter supports the PDF engines poppler and MuPDF.
# The PDF engine should be selected on the command line by compiling with
# one of these commands:
#
#    qmake RENDERER=poppler && make
# or
#    qmake RENDERER=mupdf && make
#
# The option RENDERER=both is also possible, but not recommended since in this
# case the poppler engine may fail to open some documents. You may add your
# favorite settings to make (like -j4).

### General remarks
# Library paths and linking arguments for the PDF engines are defined
# below in section "CONFIGURE LIBRARIES AND LINKING - OS DEPENDENT".
# For systems other than GNU+Linux these need to be specified manually.
# If you find a configuration for your system, you are welcome to share it!
# Just open a pull request or issue on github.

### Dependencies
# Poppler requires poppler-qt5 libraries.
# Tested with poppler 21.03.0 and 0.86.1, versions below 0.70 are not
# supported and will most probably never be supported.

# MuPDF requires mupdf libraries (libmupdf).
# Tested with libmupdf 1.18.0 and 1.16.1

equals(RENDERER, "poppler") {
    DEFINES += INCLUDE_POPPLER
} else:equals(RENDERER, "mupdf") {
    DEFINES += INCLUDE_MUPDF
} else:equals(RENDERER, "both") {
    DEFINES += INCLUDE_MUPDF
    DEFINES += INCLUDE_POPPLER
} else {
    error("You must specify which PDF engine to use as explained in beamerpresenter.pro")
}

# App name
TARGET = beamerpresenter
# Define a path where the app icon will be placed. Don't forget the trailing /.
unix: APP_ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"
macx: APP_ICON_PATH = "/usr/share/icons/hicolor/scalable/apps/"
win32: APP_ICON_PATH = ""
# Define a path where other icons will be placed. Don't forget the trailing /.
unix: ICON_PATH = "/usr/share/beamerpresenter/icons/"
macx: ICON_PATH = "/usr/share/beamerpresenter/icons/"
win32: ICON_PATH = ""
# Define the system-wide path for config files. Don't forget the trailing /.
unix: DEFAULT_CONFIG_PATH = "/etc/xdg/$${TARGET}/"
macx: DEFAULT_CONFIG_PATH = "/etc/xdg/$${TARGET}/"
win32: DEFAULT_CONFIG_PATH = ""
# Define path to fallback GUI configuration file.
GUI_CONFIG_FILE = "${{DEFAULT_CONFIG_PATH}}gui.json"

# Disable debugging message if debugging mode is disabled.
CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

# Directly using GStreamer is not implemented, but was just an experiment!
# More precisely: you might see one video in one view playing exactly once.
# And you should expect all kinds of errors.
#DEFINES += USE_QT_GSTREAMER


###########################################################
###   CONFIGURE QT + PREPROCESSOR OPTIONS
###########################################################

# Check Qt version.
# Build in multimedia support is not available in Qt 6.0, but should be
# available in Qt 6.2, expected in 2021-09.
requires(equals(QT_MAJOR_VERSION, 5))

# Check whether a PDF engine was defined.
requires(contains(DEFINES, INCLUDE_POPPLER) | contains(DEFINES, INCLUDE_MUPDF))

# Use modern C++
CONFIG += c++20 qt

# Include some libraries
QT += core gui multimedia multimediawidgets xml widgets

# Use a Qt template. Probably does something reasonable.
TEMPLATE = app

# Warn when deprecated features of Qt are used.
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
DEFINES += DEFAULT_ICON_PATH=\\\"$${ICON_PATH}\\\"
DEFINES += ICON_FILEPATH=\\\"$${APP_ICON_PATH}beamerpresenter.svg\\\"
# Define the default GUI path string.
DEFINES += DEFAULT_GUI_CONFIG_PATH=\\\"$${GUI_CONFIG_PATH}\\\"

unix {
    # Enable better debugging.
    CONFIG(debug, debug|release):QMAKE_LFLAGS += -rdynamic
}


###########################################################
###   DEFINE SOURCES
###########################################################

SOURCES += \
        src/drawing/pixmapgraphicsitem.cpp \
        src/drawing/pointingtool.cpp \
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
        src/drawing/textgraphicsitem.h \
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
    LIBS += -L/usr/lib/ -lz
    contains(DEFINES, INCLUDE_POPPLER) {
        INCLUDEPATH += /usr/include/poppler/qt5/
        LIBS += -lpoppler-qt5
    }
    contains(DEFINES, INCLUDE_MUPDF) {
        INCLUDEPATH += /usr/include/mupdf/
        LIBS += -lmupdf -lmupdf-third -lm -lfreetype -lharfbuzz -ljpeg -ljbig2dec -lopenjp2 -lgumbo
        # In my installation of MuPDF, the gumbo library is needed since MuPDF 1.18.0.
        # All other libraries were also required in MuPDF 1.17.0.
    }
    contains(DEFINES, USE_QT_GSTREAMER) {
        INCLUDEPATH += /usr/include/Qt5GStreamer/
        LIBS += -lQt5GLib-2.0 -lQt5GStreamer-1.0 -lQt5GStreamerUi-1.0
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

    # gzip man pages (1) and place them in /usr/share/man/man1
    man1.path = /usr/share/man/man1/
    man1.CONFIG = no_check_exist no_build
    man1.extra = gzip -kf9 doc/beamerpresenter.1; gzip -kf9 doc/beamerpresenter-ui.1
    man1.files = doc/beamerpresenter.1.gz doc/beamerpresenter-ui.1.gz

    # gzip man pages (5) and place them in /usr/share/man/man5
    man5.path = /usr/share/man/man5/
    man5.CONFIG = no_check_exist no_build
    man5.extra = gzip -kf9 doc/beamerpresenter.conf.5
    man5.files = doc/beamerpresenter.conf.5.gz

    # copy config/beamerpresenter.conf and config/gui.json to DEFAULT_CONFIG_PATH
    configuration.path = $${DEFAULT_CONFIG_PATH}
    configuration.CONFIG = no_build
    configuration.files = config/beamerpresenter.conf config/gui.json

    # copy app icon share/icons/beamerpresenter.svg to APP_ICON_PATH
    appicon.path = $${APP_ICON_PATH}
    appicon.CONFIG = no_build
    appicon.files = share/icons/beamerpresenter.svg

    # copy share/icons/tools (internally required icons) to $$ICON_PATH/tools
    icon.path = $${ICON_PATH}
    icon.CONFIG = no_build
    icon.files = share/icons/tools

    # copy share/doc/README.html (in-app manual) to /usr/share/doc/beamerpresenter/
    doc.path = /usr/share/doc/beamerpresenter/
    doc.CONFIG = no_build
    doc.files = share/doc/README.html

    # copy example configuration files to /usr/share/doc/beamerpresenter/examples/
    examples.path = /usr/share/doc/beamerpresenter/examples/
    examples.CONFIG = no_build
    examples.files = config/*.json

    # copy desktop file share/applications/beamerpresenter.desktop to /usr/share/applications/
    desktop.path = /usr/share/applications/
    desktop.CONFIG = no_build
    desktop.files = share/applications/beamerpresenter.desktop

    # executable should be placed in /usr/bin/
    target.path = /usr/bin/

    INSTALLS += man1 man5 configuration appicon icon doc desktop examples target
}
