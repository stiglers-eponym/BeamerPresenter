# SPDX-FileCopyrightText: 2023 Valentin Bruch <software@vbruch.eu>
# SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

configure_file(config.h.in config.h @ONLY)
add_subdirectory(rendering)

if (DEFINED MUPDF_INCLUDE_DIR)
    include_directories("${MUPDF_INCLUDE_DIR}")
endif()
if (DEFINED ZLIB_INCLUDE_DIR)
    include_directories("${ZLIB_INCLUDE_DIR}")
endif()

add_executable(beamerpresenter
        drawing/pixmapgraphicsitem.h drawing/pixmapgraphicsitem.cpp
        drawing/tool.h drawing/tool.cpp
        drawing/drawtool.h
        drawing/texttool.h
        drawing/selectiontool.h drawing/selectiontool.cpp
        drawing/pointingtool.h drawing/pointingtool.cpp
        drawing/selectionrectitem.h drawing/selectionrectitem.cpp
        drawing/textgraphicsitem.h drawing/textgraphicsitem.cpp
        drawing/rectgraphicsitem.h drawing/rectgraphicsitem.cpp
        drawing/ellipsegraphicsitem.h drawing/ellipsegraphicsitem.cpp
        drawing/arrowgraphicsitem.h drawing/arrowgraphicsitem.cpp
        drawing/linegraphicsitem.h drawing/linegraphicsitem.cpp
        drawing/flexgraphicslineitem.h
        drawing/shaperecognizer.h drawing/shaperecognizer.cpp
        drawing/pathcontainer.h drawing/pathcontainer.cpp
        drawing/abstractgraphicspath.h drawing/abstractgraphicspath.cpp
        drawing/basicgraphicspath.h drawing/basicgraphicspath.cpp
        drawing/fullgraphicspath.h drawing/fullgraphicspath.cpp
        drawing/graphicspictureitem.h
        gui/actionbutton.h gui/actionbutton.cpp
        gui/searchwidget.h gui/searchwidget.cpp
        gui/clockwidget.h gui/clockwidget.cpp
        gui/analogclockwidget.h gui/analogclockwidget.cpp
        gui/flexlayout.h gui/flexlayout.cpp
        gui/keyinputlabel.h gui/keyinputlabel.cpp
        gui/noteswidget.h gui/noteswidget.cpp
        gui/settingswidget.h gui/settingswidget.cpp
        gui/slidelabelwidget.h gui/slidelabelwidget.cpp
        gui/slidenumberwidget.h gui/slidenumberwidget.cpp
        gui/stackedwidget.h
        gui/tabwidget.h gui/tabwidget.cpp
        gui/thumbnailbutton.h gui/thumbnailbutton.cpp
        gui/thumbnailthread.h gui/thumbnailthread.cpp
        gui/thumbnailwidget.h gui/thumbnailwidget.cpp
        gui/timerwidget.h gui/timerwidget.cpp
        gui/tocbutton.h gui/tocbutton.cpp
        gui/tocwidget.h gui/tocwidget.cpp
        gui/toolbutton.h gui/toolbutton.cpp
        gui/toolselectorbutton.h gui/toolselectorbutton.cpp
        gui/toolwidgetbutton.h gui/toolwidgetbutton.cpp
        gui/tooldialog.h gui/tooldialog.cpp
        gui/tooliconengine.h gui/tooliconengine.cpp
        gui/peniconengine.h
        gui/brushiconengine.h
        gui/toolselectorwidget.h gui/toolselectorwidget.cpp
        gui/toolpropertybutton.h gui/toolpropertybutton.cpp
        gui/shapeselectionbutton.h gui/shapeselectionbutton.cpp
        gui/widthselectionbutton.h gui/widthselectionbutton.cpp
        gui/penstylebutton.h gui/penstylebutton.cpp
        gui/brushstylebutton.h gui/brushstylebutton.cpp
        gui/colorselectionbutton.h gui/colorselectionbutton.cpp
        gui/iconlabel.h
        gui/containerbaseclass.h
        gui/containerwidget.h
        gui/toolwidget.h gui/toolwidget.cpp
        rendering/abstractrenderer.h
        rendering/pdfdocument.h
        rendering/pixcache.h rendering/pixcache.cpp
        rendering/pixcachethread.h rendering/pixcachethread.cpp
        rendering/pngpixmap.h rendering/pngpixmap.cpp
        media/mediaplayer.h media/mediaplayer.cpp
        media/mediaannotation.h media/mediaannotation.cpp
        media/mediaitem.h media/mediaitem.cpp
        media/mediaslider.h
        slidescene.h slidescene.cpp
        slideview.h slideview.cpp
        pdfmaster.h pdfmaster.cpp
        master.h master.cpp
        preferences.h preferences.cpp
        enumerates.h
        names.h names.cpp
        log.h
        main.cpp
        ${EXTRA_INCLUDE}
    )


set(ZLIB_LIBRARY "z" CACHE STRING "zlib library file")
list(APPEND EXTRA_LIBS "${ZLIB_LIBRARY}")
if (USE_QTPDF)
    # TODO: This is probably not required in 5.14 > Qt >= 5.10
    list(APPEND EXTRA_LIBS "Qt${QT_VERSION_MAJOR}::Pdf")
endif()

target_link_libraries(beamerpresenter PRIVATE
        "Qt${QT_VERSION_MAJOR}::Core"
        "Qt${QT_VERSION_MAJOR}::Gui"
        "Qt${QT_VERSION_MAJOR}::Widgets"
        "Qt${QT_VERSION_MAJOR}::Multimedia"
        "Qt${QT_VERSION_MAJOR}::MultimediaWidgets"
        "Qt${QT_VERSION_MAJOR}::Xml"
        "Qt${QT_VERSION_MAJOR}::Svg"
        ${EXTRA_LIBS}
    )

target_include_directories(beamerpresenter PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}"
    )

install(TARGETS beamerpresenter RUNTIME)
