if (USE_MUPDF)
    set(DEFAULT_RENDERER mupdf)
else()
    set(DEFAULT_RENDERER poppler)
endif()
configure_file(beamerpresenter.conf.in beamerpresenter.conf)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/beamerpresenter.conf"
    "${CMAKE_CURRENT_SOURCE_DIR}/gui.json"
    DESTINATION "${GUI_CONFIG_PATH}")
install(FILES
    gui.json
    gui-2files.json
    gui-interactive.json
    DESTINATION "${DOC_PATH}/examples")
