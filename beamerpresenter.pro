#-------------------------------------------------
#
# Project created by QtCreator 2018-12-23T10:43:06
#
#-------------------------------------------------

QT       += core gui multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = beamerpresenter
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        src/main.cpp \
        src/controlscreen.cpp \
        src/pdfdoc.cpp \
        src/presentationscreen.cpp \
        src/pagelabel.cpp \
        src/clocklabel.cpp \
        src/timer.cpp \
        src/pagenumberedit.cpp \
        src/videowidget.cpp \
        src/mediaslider.cpp \
        src/pidwidcaller.cpp \
        src/cacheupdatethread.cpp

HEADERS += \
        src/controlscreen.h \
        src/pdfdoc.h \
        src/presentationscreen.h \
        src/pagelabel.h \
        src/clocklabel.h \
        src/timer.h \
        src/pagenumberedit.h \
        src/videowidget.h \
        src/mediaslider.h \
        src/pidwidcaller.h \
        src/cacheupdatethread.h

FORMS += \
        src/controlscreen.ui

INCLUDEPATH += /usr/include/poppler/qt5
LIBS += -L/usr/lib/ -lpoppler-qt5

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
