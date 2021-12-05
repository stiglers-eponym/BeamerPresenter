install(FILES
    doc/README.html
    DESTINATION "${DOC_PATH}"
    OPTIONAL)
install(FILES
    applications/beamerpresenter.desktop
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications"
    OPTIONAL)
install(FILES
    icons/beamerpresenter.svg
    DESTINATION "${APPICON_PATH}"
    OPTIONAL)
install(DIRECTORY
    icons/tools
    icons/actions
    DESTINATION "${DEFAULT_ICON_PATH}")
