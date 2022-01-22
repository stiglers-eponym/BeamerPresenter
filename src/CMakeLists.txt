configure_file(config.h.in config.h)
add_subdirectory(rendering)

add_executable(beamerpresenter
        drawing/pixmapgraphicsitem.cpp
        drawing/pointingtool.cpp
        drawing/textgraphicsitem.cpp
        drawing/rectgraphicsitem.cpp
        drawing/ellipsegraphicsitem.cpp
        gui/actionbutton.cpp
        gui/clockwidget.cpp
        gui/flexlayout.cpp
        gui/keyinputlabel.cpp
        gui/noteswidget.cpp
        gui/settingswidget.cpp
        gui/slidelabelwidget.cpp
        gui/slidenumberwidget.cpp
        gui/stackedwidget.cpp
        gui/tabwidget.cpp
        gui/thumbnailbutton.cpp
        gui/thumbnailthread.cpp
        gui/thumbnailwidget.cpp
        gui/timerwidget.cpp
        gui/tocbutton.cpp
        gui/tocwidget.cpp
        gui/toolbutton.cpp
        gui/tooldialog.cpp
        gui/toolselectorwidget.cpp
        gui/mediaslider.h
        gui/containerwidget.h
        main.cpp
        master.cpp
        pdfmaster.cpp
        preferences.cpp
        rendering/externalrenderer.cpp
        rendering/mediaplayer.cpp
        rendering/pdfdocument.cpp
        rendering/pixcache.cpp
        rendering/pixcachethread.cpp
        rendering/pngpixmap.cpp
        drawing/pathcontainer.cpp
        drawing/basicgraphicspath.cpp
        drawing/fullgraphicspath.cpp
        slidescene.cpp
        slideview.cpp
        ${EXTRA_INCLUDE}
    )

target_link_libraries(beamerpresenter PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        Qt${QT_VERSION_MAJOR}::Xml
        z
        ${EXTRA_LIBS}
    )

target_include_directories(beamerpresenter PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}"
    )

install(TARGETS beamerpresenter RUNTIME)
