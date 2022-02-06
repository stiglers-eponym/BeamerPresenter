if (USE_MUPDF)
  set(HTML_LICENSE_STRING "<p>This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the <a href=\"https://www.gnu.org/licenses/agpl-3.0.html\">GNU website</a>, or (at your choice) any newer version of the same license. This license is chosen in order to ensure compatibility with the license for MuPDF.</p>")
else()
  set(HTML_LICENSE_STRING "<p>This version of BeamerPresenter, compiled without MuPDF, may be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3, available on the <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU website</a>, or (at your choice) any newer version of the same license.</p><p>Every version of BeamerPresenter may be redistributed and/or modified under the terms of the <a href=\"https://www.gnu.org/licenses/agpl-3.0.html\">GNU Affero General Public License (AGPL)</a>, version 3 or (at your choice) any newer version of the same license..</p>")
endif()

configure_file(doc/README.html.in doc/README.html @ONLY)
install(FILES
    "${PROJECT_BINARY_DIR}/share/doc/README.html"
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
