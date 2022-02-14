if (USE_MUPDF)
    # Configure MuPDF libraries
    set(MUPDF_LIBS "${MUPDF_LIB_PATH}" m)
    if (MUPDF_USE_SYSTEM_LIBS)
        list(APPEND MUPDF_LIBS
                    freetype
                    harfbuzz
                    jpeg
                    jbig2dec
                    openjp2)
    endif()
    if (USE_MUJS)
        list(APPEND MUPDF_LIBS mujs)
    else()
        list(APPEND MUPDF_LIBS "${MUPDF_THIRD_LIB_PATH}")
    endif()
    if (USE_GUMBO)
        list(APPEND MUPDF_LIBS gumbo)
    endif()
    list(APPEND EXTRA_INCLUDE
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfdocument.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/mupdfrenderer.cpp"
        )
    list(APPEND EXTRA_LIBS ${MUPDF_LIBS})
endif()
if (USE_POPPLER)
    list(APPEND EXTRA_INCLUDE
        "${CMAKE_CURRENT_SOURCE_DIR}/popplerdocument.cpp"
        )
    list(APPEND EXTRA_LIBS
            poppler-qt${QT_VERSION_MAJOR}
        )
endif()

set(EXTRA_LIBS ${EXTRA_LIBS} PARENT_SCOPE)
set(EXTRA_INCLUDE ${EXTRA_INCLUDE} PARENT_SCOPE)
