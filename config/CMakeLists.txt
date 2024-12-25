# SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
# SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

if (USE_MUPDF)
    set(DEFAULT_RENDERER mupdf)
else()
    set(DEFAULT_RENDERER poppler)
endif()
configure_file(beamerpresenter.conf.in beamerpresenter.conf @ONLY)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/beamerpresenter.conf"
    "${CMAKE_CURRENT_SOURCE_DIR}/gui.json"
    DESTINATION "${GUI_CONFIG_PATH}")
install(FILES
    gui.json
    gui-2files.json
    gui-interactive.json
    gui-lecture.json
    DESTINATION "${DOC_PATH}/examples")
