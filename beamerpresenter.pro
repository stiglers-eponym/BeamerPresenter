#-------------------------------------------------
#
# Project created by QtCreator 2018-12-23T10:43:06
#
#-------------------------------------------------

VERSION = 0.1.0

# Check Qt version.
requires(greaterThan(QT_MAJOR_VERSION, 4))
equals(QT_MAJOR_VERSION, 4) {
    requires(greaterThan(QT_MINOR_VERSION, 5))
    smallerThan(QT_MINOR_VERSION, 9):message("Using Qt version < 5.9 is untested!.")
}

QT += core gui multimedia multimediawidgets xml widgets


TARGET = beamerpresenter
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# Define the application version.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
# If the following define is uncommented, BeamerPresenter will check whether a compatible QPA platform is used.
# It will then emit warning on untested systems and try to avoid blocking you window manager.
# If this is commented out, all checks will be omitted.
DEFINES += CHECK_QPA_PLATFORM

CONFIG += c++14 qt
unix {
    # Enable better debugging.
    CONFIG(debug):QMAKE_LFLAGS += -rdynamic
    # Enable embedded applications. This allows for X-embedding of external applications if running in X11.
    DEFINES += EMBEDDED_APPLICATIONS_ENABLED
}
linux {
    # Drop down menus can cause problems in wayland.
    # This defines activates a patch to replace drop down menus in wayland.
    DEFINES += USE_WAYLAND_SUBMENU_PATCH
}

# Disable debugging message if debugging mode is disabled.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

SOURCES += \
        src/cachemap.cpp \
        src/cachethread.cpp \
        src/main.cpp \
        src/pdfdoc.cpp \
        src/previewslide.cpp \
        src/mediaslide.cpp \
        src/drawslide.cpp \
        src/presentationslide.cpp \
        src/controlscreen.cpp \
        src/presentationscreen.cpp \
        src/timer.cpp \
        src/pagenumberedit.cpp \
        src/toolbutton.cpp \
        src/toolselector.cpp \
        src/videowidget.cpp \
        src/tocbox.cpp \
        src/tocbutton.cpp \
        src/tocaction.cpp \
        src/externalrenderer.cpp \
        src/overviewframe.cpp \
        src/overviewbox.cpp \
        src/drawpath.cpp

HEADERS += \
        src/cachemap.h \
        src/cachethread.h \
        src/enumerates.h \
        src/pdfdoc.h \
        src/previewslide.h \
        src/mediaslide.h \
        src/drawslide.h \
        src/controlscreen.h \
        src/presentationscreen.h \
        src/presentationslide.h \
        src/timer.h \
        src/pagenumberedit.h \
        src/toolbutton.h \
        src/toolselector.h \
        src/videowidget.h \
        src/tocbox.h \
        src/tocbutton.h \
        src/tocaction.h \
        src/externalrenderer.h \
        src/overviewframe.h \
        src/overviewbox.h \
        src/drawpath.h

contains(DEFINES, EMBEDDED_APPLICATIONS_ENABLED) {
    SOURCES += src/embedapp.cpp
    HEADERS += src/embedapp.h
}

FORMS += \
        src/controlscreen.ui

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
    doc1.path = /usr/share/man/man1/
    doc1.extra = ls $${TARGET}.1.gz || gzip -9 $${TARGET}.1; $(INSTALL_FILE) $${PWD}/$${TARGET}.1.gz $(INSTALL_ROOT)/usr/share/man/man1/$${TARGET}.1.gz
    #doc1.files = $${TARGET}.1.gz # doesn't work

    doc5.path = /usr/share/man/man5/
    doc5.extra = ls $${TARGET}.conf.5.gz || gzip -9 $${TARGET}.conf.5; $(INSTALL_FILE) $${PWD}/$${TARGET}.conf.5.gz $(INSTALL_ROOT)/usr/share/man/man1/$${TARGET}.conf.5.gz
    #doc5.files = $${TARGET}.conf.5.gz # doesn't work

    configuration.path = /etc/$${TARGET}/
    configuration.files = $${TARGET}.conf pid2wid.sh

    icon.path = /usr/share/icons/scalable/apps/
    icon.files = src/beamerpresenter.svg

    target.path = /usr/bin/

    INSTALLS += doc1 doc5 configuration icon target
}
