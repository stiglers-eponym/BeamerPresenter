cmake_minimum_required(VERSION 3.16)

project(beamerpresenter
        VERSION 0.2.1
        DESCRIPTION "Multi-screen PDF presentation viewer"
        HOMEPAGE_URL "https://github.com/stiglers-eponym/BeamerPresenter"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
if (VERSION VERSION_LESS 3.22)
    set(CMAKE_CONFIGURATION_TYPES Release Debug)
else()
    set(CMAKE_CONFIGURATION_TYPES Release Debug MinSizeRel RelWithDebInfo)
endif()
set(QT_VERSION_MAJOR 6 CACHE STRING "Qt major version")
set(QT "Qt${QT_VERSION_MAJOR}")

find_package(${QT} COMPONENTS Core REQUIRED)
find_package(${QT} COMPONENTS Gui REQUIRED)
find_package(${QT} COMPONENTS Widgets REQUIRED)
find_package(${QT} COMPONENTS Multimedia REQUIRED)
find_package(${QT} COMPONENTS MultimediaWidgets REQUIRED)
find_package(${QT} COMPONENTS Xml REQUIRED)

option(USE_MUPDF "Include MuPDF" ON)
option(USE_POPPLER "Include Poppler" OFF)
option(CREATE_SHARED_LIBRARIES "Put the PDF engine in a shared library" OFF)

if ((NOT USE_MUPDF) AND (NOT USE_POPPLER))
    message(FATAL_ERROR "No PDF engine provided")
elseif (USE_MUPDF AND USE_POPPLER)
    message(WARNING "Two PDF engine provided, Poppler will not work for some documents")
endif()

set(APP_VERSION "${beamerpresenter_VERSION}")
if (EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND git -C ${PROJECT_SOURCE_DIR} rev-list --count HEAD
        COMMAND head -c-1
        TIMEOUT 1
        OUTPUT_VARIABLE APP_GIT_COMMITS)
    execute_process(
        COMMAND git -C ${PROJECT_SOURCE_DIR} rev-parse --short HEAD
        COMMAND head -c-1
        TIMEOUT 1
        OUTPUT_VARIABLE APP_GIT_HASH)
    message(NOTICE "Set version: ${APP_VERSION} ${APP_GIT_COMMITS} ${APP_GIT_HASH}")
    if (APP_GIT_COMMITS AND APP_GIT_HASH)
        set(APP_VERSION "${beamerpresenter_VERSION}-${APP_GIT_COMMITS}.${APP_GIT_HASH}")
    endif()
endif()

set(LFLAGS "${LFLAGS} $<$<CONFIG:Debug>:-rdynamic>>")


include(GNUInstallDirs)

set(DEFAULT_ICON_PATH "${CMAKE_INSTALL_DATAROOTDIR}/beamerpresenter/icons" CACHE STRING)
GNUInstallDirs_get_absolute_install_dir(ABS_ICON_PATH DEFAULT_ICON_PATH DATAROOTDIR)
set(ABS_ICON_PATH "${ABS_ICON_PATH}/")

set(APPICON_PATH "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps" CACHE STRING)
GNUInstallDirs_get_absolute_install_dir(ABS_APPICON_PATH APPICON_PATH DATAROOTDIR)
set(ABS_APPICON_PATH "${ABS_APPICON_PATH}/beamerpresenter.svg")

set(GUI_CONFIG_PATH "${CMAKE_INSTALL_SYSCONFDIR}/xdg/beamerpresenter" CACHE STRING)
GNUInstallDirs_get_absolute_install_dir(ABS_GUI_CONFIG_PATH GUI_CONFIG_PATH SYSCONFDIR)
set(ABS_GUI_CONFIG_PATH "${ABS_GUI_CONFIG_PATH}/gui.json")

set(DOC_PATH "${CMAKE_INSTALL_DOCDIR}" CACHE STRING)
GNUInstallDirs_get_absolute_install_dir(ABS_DOC_PATH DOC_PATH DOCDIR)
set(ABS_DOC_PATH "${ABS_DOC_PATH}/README.html")

add_subdirectory(config)
add_subdirectory(share)
add_subdirectory(man)
add_subdirectory(src)

if (VERSION VERSION_LESS 3.22)
    target_compile_definitions(beamerpresenter PUBLIC
            $<$<CONFIG:Debug>:QT_DEBUG>
            $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:Release>:QT_NO_DEBUG>
            $<$<CONFIG:Debug>:QT_DEPRECATED_WARNINGS>
            $<$<CONFIG:Debug>:FITZ_DEBUG_LOCKING>
        )
else()
    target_compile_definitions(beamerpresenter PUBLIC
            $<$<CONFIG:Debug,RelWithDebInfo>:QT_DEBUG>
            $<$<CONFIG:Release,MinSizeRel>:QT_NO_DEBUG_OUTPUT>
            $<$<CONFIG:Release,MinSizeRel>:QT_NO_DEBUG>
            $<$<CONFIG:Debug,RelWithDebInfo>:QT_DEPRECATED_WARNINGS>
            $<$<CONFIG:Debug,RelWithDebInfo>:FITZ_DEBUG_LOCKING>
        )
endif()
