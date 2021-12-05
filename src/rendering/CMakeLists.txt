if (CREATE_SHARED_LIBRARIES)
    if (USE_MUPDF)
        add_library(beamerpresenter-pdf-mupdf SHARED
                #pngpixmap.cpp
                pdfdocument.cpp
                mupdfdocument.cpp
                mupdfrenderer.cpp
            )
        target_link_libraries(beamerpresenter-pdf-mupdf PUBLIC
                ${QT}::Core
                ${QT}::Gui
                ${QT}::Widgets
                mupdf
                mupdf-third
                m
                freetype
                harfbuzz
                jpeg
                jbig2dec
                openjp2
                gumbo
            )
        target_include_directories(beamerpresenter-pdf-mupdf PUBLIC
                "${PROJECT_BINARY_DIR}"
                "${PROJECT_SOURCE_DIR}"
            )
        list(APPEND EXTRA_LIBS
                beamerpresenter-pdf-mupdf
            )
        install(TARGETS beamerpresenter-pdf-mupdf LIBRARY)
    endif()
    if (USE_POPPLER)
        add_library(beamerpresenter-pdf-poppler SHARED
                #pngpixmap.cpp
                pdfdocument.cpp
                popplerdocument.cpp
            )
        target_link_libraries(beamerpresenter-pdf-poppler PUBLIC
                ${QT}::Core
                ${QT}::Gui
                ${QT}::Widgets
                poppler-qt${QT_VERSION_MAJOR}
            )
        target_include_directories(beamerpresenter-pdf-poppler PUBLIC
                "${PROJECT_BINARY_DIR}"
                "${PROJECT_SOURCE_DIR}"
            )
        list(APPEND EXTRA_LIBS
                beamerpresenter-pdf-poppler
            )
        install(TARGETS beamerpresenter-pdf-poppler LIBRARY)
    endif()
else()
    if (USE_MUPDF)
        list(APPEND EXTRA_INCLUDE
                "${CMAKE_CURRENT_SOURCE_DIR}/mupdfdocument.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/mupdfrenderer.cpp"
            )
        list(APPEND EXTRA_LIBS
                mupdf
                mupdf-third
                m
                freetype
                harfbuzz
                jpeg
                jbig2dec
                openjp2
                gumbo
            )
    endif()
    if (USE_POPPLER)
        list(APPEND EXTRA_INCLUDE
            "${CMAKE_CURRENT_SOURCE_DIR}/popplerdocument.cpp"
            )
        list(APPEND EXTRA_LIBS
                poppler-qt${QT_VERSION_MAJOR}
            )
    endif()
endif()

set(EXTRA_LIBS ${EXTRA_LIBS} PARENT_SCOPE)
set(EXTRA_INCLUDE ${EXTRA_INCLUDE} PARENT_SCOPE)
