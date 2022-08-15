# SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
# SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

set(MAN_FILES beamerpresenter.1 beamerpresenter-ui.5 beamerpresenter.conf.5)

set(MAN1_TARGETS)
set(MAN5_TARGETS)
foreach(MANPAGE IN LISTS MAN_FILES)
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}.gz"
        COMMAND gzip -c9 "${CMAKE_CURRENT_SOURCE_DIR}/${MANPAGE}" > "${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}.gz"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${MANPAGE}"
        VERBATIM
        )
    if(MANPAGE MATCHES ".*\\.1$")
        list(APPEND MAN1_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}.gz")
    elseif(MANPAGE MATCHES ".*\\.5$")
        list(APPEND MAN5_TARGETS "${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}.gz")
    endif()
endforeach()
add_custom_target(man ALL DEPENDS ${MAN1_TARGETS} ${MAN5_TARGETS})

install(FILES
    ${MAN1_TARGETS}
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
install(FILES
    ${MAN5_TARGETS}
    DESTINATION "${CMAKE_INSTALL_MANDIR}/man5")
