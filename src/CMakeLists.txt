configure_file(config.h.in config.h)
add_subdirectory(rendering)

if (DEFINED MUPDF_INCLUDE_DIR)
    include_directories("${MUPDF_INCLUDE_DIR}")
endif()
if (DEFINED ZLIB_INCLUDE_DIR)
    include_directories("${ZLIB_INCLUDE_DIR}")
endif()

add_executable(beamerpresenter
        drawing/pixmapgraphicsitem.h drawing/pixmapgraphicsitem.cpp
        drawing/pointingtool.h drawing/pointingtool.cpp
        drawing/textgraphicsitem.h drawing/textgraphicsitem.cpp
        drawing/rectgraphicsitem.h drawing/rectgraphicsitem.cpp
        drawing/ellipsegraphicsitem.h drawing/ellipsegraphicsitem.cpp
        drawing/shaperecognizer.h drawing/shaperecognizer.cpp
        drawing/pathcontainer.h drawing/pathcontainer.cpp
        drawing/basicgraphicspath.h drawing/basicgraphicspath.cpp
        drawing/fullgraphicspath.h drawing/fullgraphicspath.cpp
        gui/actionbutton.h gui/actionbutton.cpp
        gui/clockwidget.h gui/clockwidget.cpp
        gui/flexlayout.h gui/flexlayout.cpp
        gui/keyinputlabel.h gui/keyinputlabel.cpp
        gui/noteswidget.h gui/noteswidget.cpp
        gui/settingswidget.h gui/settingswidget.cpp
        gui/slidelabelwidget.h gui/slidelabelwidget.cpp
        gui/slidenumberwidget.h gui/slidenumberwidget.cpp
        gui/stackedwidget.h gui/stackedwidget.cpp
        gui/tabwidget.h gui/tabwidget.cpp
        gui/thumbnailbutton.h gui/thumbnailbutton.cpp
        gui/thumbnailthread.h gui/thumbnailthread.cpp
        gui/thumbnailwidget.h gui/thumbnailwidget.cpp
        gui/timerwidget.h gui/timerwidget.cpp
        gui/tocbutton.h gui/tocbutton.cpp
        gui/tocwidget.h gui/tocwidget.cpp
        gui/toolbutton.h gui/toolbutton.cpp
        gui/tooldialog.h gui/tooldialog.cpp
        gui/toolselectorwidget.h gui/toolselectorwidget.cpp
        gui/toolpropertybutton.h gui/toolpropertybutton.cpp
        gui/shapeselectionbutton.h gui/shapeselectionbutton.cpp
        gui/penstylebutton.h gui/penstylebutton.cpp
        gui/colorselectionbutton.h gui/colorselectionbutton.cpp
        gui/mediaslider.h
        gui/containerwidget.h
        rendering/externalrenderer.h rendering/externalrenderer.cpp
        rendering/mediaplayer.h rendering/mediaplayer.cpp
        rendering/pdfdocument.h rendering/pdfdocument.cpp
        rendering/pixcache.h rendering/pixcache.cpp
        rendering/pixcachethread.h rendering/pixcachethread.cpp
        rendering/pngpixmap.h rendering/pngpixmap.cpp
        slidescene.h slidescene.cpp
        slideview.h slideview.cpp
        pdfmaster.h pdfmaster.cpp
        master.h master.cpp
        preferences.h preferences.cpp
        main.cpp
        ${EXTRA_INCLUDE}
    )

if (DEFINED "${ZLIB_LIBRARY}")
    list(APPEND EXTRA_LIBS "${ZLIB_LIBRARY}")
else()
    list(APPEND EXTRA_LIBS "z")
endif()

target_link_libraries(beamerpresenter PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Multimedia
        Qt${QT_VERSION_MAJOR}::MultimediaWidgets
        Qt${QT_VERSION_MAJOR}::Xml
        ${EXTRA_LIBS}
    )

target_include_directories(beamerpresenter PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}"
    )

install(TARGETS beamerpresenter RUNTIME)
