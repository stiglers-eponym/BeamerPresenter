cmake_minimum_required(VERSION 3.16)

project(beamerpresenter
    VERSION 0.2.1
    DESCRIPTION "Multi-screen PDF presentation viewer"
    HOMEPAGE_URL "https://github.com/stiglers-eponym/BeamerPresenter"
    LANGUAGES CXX)

# Define available build/config types
set(QT_VERSION_MAJOR 6 CACHE STRING "Qt major version")
# Minor Qt version is required for defining dependencies when packaging
set(QT_VERSION_MINOR 2 CACHE STRING "Qt minor version")

# Check if the required Qt libraries are available
set(QT_FEATURE_intelcet ON)  # without the project fails to build...
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS MultimediaWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Xml REQUIRED)

# Options concerning the PDF engine
option(USE_MUPDF "Include MuPDF" ON)
option(USE_POPPLER "Include Poppler" OFF)
option(CREATE_SHARED_LIBRARIES "Put the PDF engine in a shared library" OFF)

if ((NOT USE_MUPDF) AND (NOT USE_POPPLER))
    message(FATAL_ERROR "No PDF engine provided")
elseif (USE_MUPDF AND USE_POPPLER)
    message(WARNING "Two PDF engine provided, Poppler will (probably) not work for some documents")
endif()


# Define some default paths
include(GNUInstallDirs)

set(DEFAULT_ICON_PATH
    "${CMAKE_INSTALL_DATAROOTDIR}/beamerpresenter/icons"
    CACHE STRING
    "Directory for storing icons used by the programm")
GNUInstallDirs_get_absolute_install_dir(ABS_ICON_PATH DEFAULT_ICON_PATH DATAROOTDIR)
set(ABS_ICON_PATH "${ABS_ICON_PATH}/")

set(APPICON_PATH
    "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps"
    CACHE STRING
    "Directory for storingthe app icon")
GNUInstallDirs_get_absolute_install_dir(ABS_APPICON_PATH APPICON_PATH DATAROOTDIR)
set(ABS_APPICON_PATH "${ABS_APPICON_PATH}/beamerpresenter.svg")

set(GUI_CONFIG_PATH
    "${CMAKE_INSTALL_SYSCONFDIR}/xdg/beamerpresenter"
    CACHE STRING
    "Configuration directory")
GNUInstallDirs_get_absolute_install_dir(ABS_GUI_CONFIG_PATH GUI_CONFIG_PATH SYSCONFDIR)
set(ABS_GUI_CONFIG_PATH "${ABS_GUI_CONFIG_PATH}/gui.json")

set(DOC_PATH
    "${CMAKE_INSTALL_DOCDIR}"
    CACHE STRING
    "Directory for storing the documentation and example configurations")
GNUInstallDirs_get_absolute_install_dir(ABS_DOC_PATH DOC_PATH DOCDIR)
set(ABS_DOC_PATH "${ABS_DOC_PATH}/README.html")


# Set app version string
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

# Configure compilation basics
set(CMAKE_CONFIGURATION_TYPES Release Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
# Linker flags for debugging
set(LFLAGS "${LFLAGS} $<$<CONFIG:Debug>:-rdynamic>>")

# Add subdirectory containing C++ sources. This defines target beamerpresenter.
add_subdirectory(src)

# Compiler definitions
target_compile_definitions(beamerpresenter PUBLIC
        $<$<CONFIG:Debug>:QT_DEBUG>
        $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
        $<$<CONFIG:Release>:QT_NO_DEBUG>
        $<$<CONFIG:Debug>:QT_DEPRECATED_WARNINGS>
        $<$<CONFIG:Debug>:FITZ_DEBUG_LOCKING>
    )

# Add other subdirectories.
add_subdirectory(config)
add_subdirectory(share)
add_subdirectory(man)


# Packaging
#set(CPACK_GENERATOR "DEB;" CACHE STRING "Package types (DEB requires Qt5)")
set(PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_CONTACT "Valentin Bruch <software@vbruch.eu>")
set(CPACK_PACKAGE_VERSION "${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")
if (USE_MUPDF AND USE_POPPLER)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-full")
    set(CPACK_PACKAGE_FILE_NAME "beamerpresenter-full-${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}-${PACKAGE_ARCH}")
elseif (USE_MUPDF)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-mupdf")
    set(CPACK_PACKAGE_FILE_NAME "beamerpresenter-mupdf-${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}-${PACKAGE_ARCH}")
elseif(USE_POPPLER)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-poppler")
    set(CPACK_PACKAGE_FILE_NAME "beamerpresenter-poppler-${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}-${PACKAGE_ARCH}")
endif()
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/package_description")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES ON)

if (CPACK_GENERATOR MATCHES "DEB")
    # configure debian package
    if (NOT ("${QT_VERSION_MAJOR}" STREQUAL "5"))
        message(ERROR, "Building debian package is only possible with Qt 5")
    endif()
    MATH(EXPR QT_VERSION_MINOR_PLUS "${QT_VERSION_MINOR} + 1")
    set(QT_VERSION_UPPER "(<< ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR_PLUS}.0)")
    set(QT_VERSION_LOWER "(>= ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.0)")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "zlib1g (>= 1:1.2.11)")
    foreach (package "libqt${QT_VERSION_MAJOR}multimedia5" "libqt${QT_VERSION_MAJOR}multimediawidgets5")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${package} ${QT_VERSION_LOWER}, ${package} ${QT_VERSION_UPPER}")
    endforeach()
    if (USE_MUPDF)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libfreetype6, libharfbuzz0b, libjpeg8, libopenjp2-7, libjbig2dec0, libgumbo1")
    endif()
    if (USE_POPPLER)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libpoppler-qt${QT_VERSION_MAJOR}-1 (>= 0.86.1)")
    endif()
    if ("${PACKAGE_ARCH}" STREQUAL "x86_64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    else()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${PACKAGE_ARCH}")
    endif()
    set(CPACK_DEBIAN_PACKAGE_SECTION "science")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "libqt5multimedia5-plugins")
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_INSTALL_DOCDIR}" RENAME "copyright")
endif()

include(CPack)
