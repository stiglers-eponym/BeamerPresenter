if (USE_MUPDF)
    # Configure MuPDF libraries
    list(APPEND EXTRA_LIBS "${MUPDF_LIB_PATH}")
    # If system libraries are used (default in most popular Linux distributions),
    # these must be added here.
    if (MUPDF_USE_SYSTEM_LIBS)
        list(APPEND EXTRA_LIBS
                    freetype
                    harfbuzz
                    jpeg
                    jbig2dec
                    openjp2)
        # Gumbo is only required since MuPDF 1.18 (for the tested Linux distributions)
        if (USE_GUMBO)
            list(APPEND EXTRA_LIBS gumbo)
        endif()
    endif()
    # In Ubuntu there is no libmupdfthird, but instead the program must be linked to MuJS.
    if (USE_MUJS)
        list(APPEND EXTRA_LIBS mujs)
    else()
        list(APPEND EXTRA_LIBS "${MUPDF_THIRD_LIB_PATH}")
    endif()
    list(APPEND EXTRA_INCLUDE
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfdocument.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfdocument.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfrenderer.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfrenderer.cpp"
        )
endif()
if (USE_POPPLER)
    list(APPEND EXTRA_INCLUDE
        "${CMAKE_CURRENT_SOURCE_DIR}/popplerdocument.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/popplerdocument.cpp"
        )
    list(APPEND EXTRA_LIBS poppler-qt${QT_VERSION_MAJOR})
endif()

set(EXTRA_LIBS ${EXTRA_LIBS} PARENT_SCOPE)
set(EXTRA_INCLUDE ${EXTRA_INCLUDE} PARENT_SCOPE)
