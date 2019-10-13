#-------------------------------------------------
#
# Project created by QtCreator 2018-12-23T10:43:06
#
#-------------------------------------------------

requires(greaterThan(QT_MAJOR_VERSION, 4))
equals(QT_MAJOR_VERSION, 4) {
	requires(greaterThan(QT_MINOR_VERSION, 5))
	smallerThan(QT_MINOR_VERSION, 9):message("Using Qt version < 5.9 is untested!.")
}

QT       += core gui multimedia multimediawidgets xml widgets

CONFIG(debug):QMAKE_LFLAGS += -rdynamic
TARGET = beamerpresenter
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += qt
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
        src/main.cpp \
        src/pdfdoc.cpp \
        src/basicslide.cpp \
        src/previewslide.cpp \
        src/mediaslide.cpp \
        src/drawslide.cpp \
        src/presentationslide.cpp \
        src/controlscreen.cpp \
        src/presentationscreen.cpp \
        src/clocklabel.cpp \
        src/timer.cpp \
        src/pagenumberedit.cpp \
        src/toolselector.cpp \
        src/videowidget.cpp \
        src/cacheupdatethread.cpp \
        src/tocbox.cpp \
        src/tocbutton.cpp \
        src/tocaction.cpp \
        src/externalrenderer.cpp \
        src/embedapp.cpp \
        src/overviewframe.cpp \
        src/overviewbox.cpp \
        src/drawpath.cpp

HEADERS += \
        src/enumerates.h \
        src/pdfdoc.h \
        src/basicslide.h \
        src/previewslide.h \
        src/mediaslide.h \
        src/drawslide.h \
        src/controlscreen.h \
        src/presentationscreen.h \
        src/clocklabel.h \
        src/presentationslide.h \
        src/timer.h \
        src/pagenumberedit.h \
        src/toolselector.h \
        src/videowidget.h \
        src/cacheupdatethread.h \
        src/tocbox.h \
        src/tocbutton.h \
        src/tocaction.h \
        src/externalrenderer.h \
        src/embedapp.h \
        src/overviewframe.h \
        src/overviewbox.h \
        src/drawpath.h

FORMS += \
        src/controlscreen.ui

unix {
    INCLUDEPATH += /usr/include/poppler/qt5
    LIBS += -L /usr/lib/ -lpoppler-qt5
}
#macx {
#	# Testing required!
#	INCLUDEPATH += /usr/include/poppler/qt5
#	LIBS += -L/usr/lib/ -lpoppler-qt5
#}
#win32 {
#	# Testing required!
#	INCLUDEPATH += /usr/include/poppler/qt5
#	LIBS += -L/usr/lib/ -lpoppler-qt5
#}

# TODO: libraries for Windows (and mac)

unix {
    documentation.path = /usr/share/man/man1
    documentation.extra = ls $${TARGET}.1.gz || gzip -9 $${TARGET}.1
    documentation.files = $${TARGET}.1.gz

    configuration.path = /etc/$${TARGET}/
    configuration.files = $${TARGET}.conf pid2wid.sh

    target.path = /usr/bin/

    INSTALLS += documentation configuration target
}
