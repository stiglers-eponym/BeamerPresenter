cmake_minimum_required(VERSION 3.16)

project(beamerpresenter
        VERSION 0.2.1
        DESCRIPTION "Multi-screen PDF presentation viewer"
        HOMEPAGE_URL "https://github.com/stiglers-eponym/BeamerPresenter"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Qt6 COMPONENTS Gui REQUIRED)
find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Multimedia REQUIRED)
find_package(Qt6 COMPONENTS MultimediaWidgets REQUIRED)
find_package(Qt6 COMPONENTS Xml REQUIRED)

option(USE_MUPDF "Include MuPDF" ON)
option(USE_POPPLER "Include Poppler" OFF)
option(CREATE_SHARED_LIBRARIES "Put the PDF engine in a shared library" OFF)

add_definitions(
        -DDEFAULT_ICON_PATH="/usr/share/beamerpresenter/icons/"
        -DICON_FILEPATH="/usr/share/icons/hicolor/scalable/apps/beamerpresenter.svg"
        -DDEFAULT_GUI_CONFIG_PATH="/etc/xdg/beamerpresenter/"
    )

add_subdirectory(src)

add_executable(beamerpresenter
        src/drawing/pixmapgraphicsitem.cpp
        src/drawing/pointingtool.cpp
        src/drawing/textgraphicsitem.cpp
        src/gui/actionbutton.cpp
        src/gui/clockwidget.cpp
        src/gui/flexlayout.cpp
        src/gui/keyinputlabel.cpp
        src/gui/noteswidget.cpp
        src/gui/settingswidget.cpp
        src/gui/slidelabelwidget.cpp
        src/gui/slidenumberwidget.cpp
        src/gui/stackedwidget.cpp
        src/gui/tabwidget.cpp
        src/gui/thumbnailbutton.cpp
        src/gui/thumbnailthread.cpp
        src/gui/thumbnailwidget.cpp
        src/gui/timerwidget.cpp
        src/gui/tocbutton.cpp
        src/gui/tocwidget.cpp
        src/gui/toolbutton.cpp
        src/gui/tooldialog.cpp
        src/gui/toolselectorwidget.cpp
        src/gui/mediaslider.h
        src/gui/containerwidget.h
        src/main.cpp
        src/master.cpp
        src/pdfmaster.cpp
        src/preferences.cpp
        src/rendering/externalrenderer.cpp
        src/rendering/mediaplayer.cpp
        src/rendering/pdfdocument.cpp
        src/rendering/pixcache.cpp
        src/rendering/pixcachethread.cpp
        src/rendering/pngpixmap.cpp
        src/drawing/pathcontainer.cpp
        src/drawing/basicgraphicspath.cpp
        src/drawing/fullgraphicspath.cpp
        src/slidescene.cpp
        src/slideview.cpp
        ${EXTRA_INCLUDE}
    )

target_link_libraries(beamerpresenter PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Xml
        z
        ${EXTRA_LIBS}
    )

target_include_directories(beamerpresenter PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}"
    )
