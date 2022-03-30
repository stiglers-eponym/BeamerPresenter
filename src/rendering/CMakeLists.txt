if (USE_MUPDF)
    # Configure MuPDF libraries
    if (DEFINED MUPDF_LIB_PATH)
        list(APPEND EXTRA_LIBS "${MUPDF_LIB_PATH}")
    else()
        list(APPEND EXTRA_LIBS mupdf)
    endif()
    if (USE_MUPDF_THIRD)
        # In Ubuntu 21.10 there is no mupdf-third.
        # In all systems with a mupdf-third library it's probably a good idea to link to it.
        if (DEFINED MUPDF_THIRD_LIB_PATH)
            list(APPEND EXTRA_LIBS "${MUPDF_THIRD_LIB_PATH}")
        else()
            list(APPEND EXTRA_LIBS mupdf-third)
        endif()
    endif()
    if (USE_MUJS)
        # In Ubuntu >= 21.10 the program must be linked to MuJS.
        list(APPEND EXTRA_LIBS mujs)
    endif()
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
