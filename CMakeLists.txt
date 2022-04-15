cmake_minimum_required(VERSION 3.16)

project(beamerpresenter
    VERSION 0.2.2
    DESCRIPTION "Multi-screen PDF presentation viewer"
    HOMEPAGE_URL "https://github.com/stiglers-eponym/BeamerPresenter"
    LANGUAGES CXX)

# Define available build/config types
set(QT_VERSION_MAJOR 6 CACHE STRING "Qt major version")
# Minor Qt version is required for defining dependencies when packaging
set(QT_VERSION_MINOR 2 CACHE STRING "Qt minor version")

# Check if the required Qt libraries are available
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Multimedia REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS MultimediaWidgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Xml REQUIRED)


# Options concerning the installation
option(GENERATE_MANPAGES "Generate man pages and include them in installation" ON)
option(INSTALL_LICENSE "Include license file in installation" ON)
option(MARK_AS_MINIMAL "Mark package as minimal (set ON when using minimal version of MuPDF)" OFF)

# Options concerning the PDF engine
option(USE_QTPDF "Include QtPDF (requires Qt >= 6.3 or Qt >= 5.10)" OFF)
option(USE_MUPDF "Include MuPDF" ON)
option(USE_POPPLER "Include Poppler" OFF)
option(USE_EXTERNAL_RENDERER "Include option to use external renderer" OFF)
option(USE_MUPDF_THIRD "Link to mupdf-third (only relevant when using MuPDF)" ON)
option(USE_MUJS "Link to mujs (only relevant when using MuPDF)" OFF)
option(USE_GUMBO "Link to gumbo-parser, should be on when using MuPDF >= 1.18" ON)
option(MUPDF_USE_SYSTEM_LIBS "MuPDF uses system libraries that need to be included. This is the default for most Linux packages of MuPDF." ON)

# Warnings and errors based on the selection of PDF engines
if ((NOT USE_MUPDF) AND (NOT USE_POPPLER))
    if (USE_QTPDF)
        message(WARNING "Only Qt PDF included as PDF engine, features will be very limited")
    else()
        message(FATAL_ERROR "No PDF engine provided")
    endif()
elseif (USE_MUPDF AND USE_POPPLER)
    message(WARNING "MuPDF and Poppler included, Poppler will (probably) not work for some documents")
endif()

if (USE_QTPDF)
    # TODO: This is probably not required in 5.14 > Qt >= 5.10
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Pdf REQUIRED)
endif()


# Define some default paths
include(GNUInstallDirs)

set(DEFAULT_ICON_PATH
    "${CMAKE_INSTALL_DATAROOTDIR}/beamerpresenter/icons"
    CACHE STRING
    "Directory for storing icons used by the programm")
GNUInstallDirs_get_absolute_install_dir(ABS_ICON_PATH DEFAULT_ICON_PATH DATAROOTDIR)

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

set(TRANSLATION_PATH
    "${CMAKE_INSTALL_DATAROOTDIR}/beamerpresenter/translations"
    CACHE STRING
    "Directory for storing internationalization/translation files")
GNUInstallDirs_get_absolute_install_dir(ABS_TRANSLATION_PATH TRANSLATION_PATH DATAROOTDIR)
set(ABS_TRANSLATION_PATH "${ABS_TRANSLATION_PATH}/")


# Set app version string
set(APP_VERSION "${beamerpresenter_VERSION}")
option(GIT_VERSION "include git commit count in version" ON)
if (GIT_VERSION AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
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
    message(NOTICE "Set version: ${APP_VERSION}-${APP_GIT_COMMITS}.${APP_GIT_HASH}")
    if (APP_GIT_COMMITS AND APP_GIT_HASH)
        set(APP_VERSION "${APP_VERSION}-${APP_GIT_COMMITS}.${APP_GIT_HASH}")
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

# Translations
option(USE_TRANSLATIONS "Enable translations" ON)
if (USE_TRANSLATIONS)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools REQUIRED)
    if (${QT_VERSION_MAJOR} EQUAL 5)
        qt5_add_translation(qm_files translations/de.ts)
        add_custom_target(translations ALL DEPENDS ${qm_files})
    else()
        qt_add_lupdate(beamerpresenter TS_FILE translations/de.ts)
        qt_add_lrelease(beamerpresenter TS_FILES translations/de.ts QM_FILES_OUTPUT_VARIABLE qm_files)
    endif()
    install(FILES ${qm_files} DESTINATION "${TRANSLATION_PATH}")
endif()

# License
if (INSTALL_LICENSE)
    install(FILES LICENSE LICENSE_GPL LICENSE_AGPL DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/licenses/beamerpresenter/")
endif()

# Add other subdirectories.
add_subdirectory(config)
add_subdirectory(share)
if (GENERATE_MANPAGES)
    add_subdirectory(man)
endif()


# Packaging
set(PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_PACKAGE_CONTACT "Valentin Bruch <software@vbruch.eu>")
set(CPACK_PACKAGE_VERSION "${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")
if (USE_MUPDF AND USE_POPPLER)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-full")
    set(CONFLICTING_PACKAGES "${PROJECT_NAME}-mupdf, ${PROJECT_NAME}-mupdf-min, ${PROJECT_NAME}-poppler, ${PROJECT_NAME}-qtpdf")
elseif (USE_MUPDF)
    if (MARK_AS_MINIMAL)
        set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-mupdf-min")
        set(CONFLICTING_PACKAGES "${PROJECT_NAME}-full, ${PROJECT_NAME}-mupdf, ${PROJECT_NAME}-poppler, ${PROJECT_NAME}-qtpdf")
    else()
        set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-mupdf")
        set(CONFLICTING_PACKAGES "${PROJECT_NAME}-full, ${PROJECT_NAME}-mupdf-min, ${PROJECT_NAME}-poppler, ${PROJECT_NAME}-qtpdf")
    endif()
elseif(USE_POPPLER)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-poppler")
    set(CONFLICTING_PACKAGES "${PROJECT_NAME}-full, ${PROJECT_NAME}-mupdf-min, ${PROJECT_NAME}-mupdf, ${PROJECT_NAME}-qtpdf")
elseif(USE_QTPDF)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}-qtpdf")
    set(CONFLICTING_PACKAGES "${PROJECT_NAME}-full, ${PROJECT_NAME}-mupdf-min, ${PROJECT_NAME}-mupdf, ${PROJECT_NAME}-poppler")
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${APP_VERSION}-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}-${PACKAGE_ARCH}")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/packaging/package_description")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_STRIP_FILES ON)

MATH(EXPR QT_VERSION_MINOR_PLUS "${QT_VERSION_MINOR} + 1")

if (CPACK_GENERATOR MATCHES "DEB")
    # configure debian package (tested in Ubuntu 20.04, Ubuntu 21.10, and partially Ubuntu 22.04)
    #
    # Required packages for building:
    # * cmake
    # * git (only when building mainline version)
    # * zlib1g-dev
    # * qtmultimedia5-dev (qt6-multimedia-dev for Qt 6)
    # * qttools5-dev (or qt6-l10n-tools for Qt 6)
    # * libqt6opengl6-dev (for Qt 6)
    # * libgl1-mesa-dev (for Qt 6)
    # For poppler:
    # * libpoppler-qt5-dev
    # For MuPDF:
    # * libmupdf-dev
    # * libfreetype-dev
    # * libharfbuzz-dev
    # * libjpeg-dev
    # * libopenjp2-7-dev
    # * libjbig2dec0-dev
    # * libgumbo-dev (for MuPDF >=1.18, probably not required for version <=1.17)
    # * libmujs-dev (in Ubuntu >=21.10)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "zlib1g (>= 1:1.2.11)")
    set(QT_MEDIA_PACKAGE "libqt${QT_VERSION_MAJOR}multimediawidgets${QT_VERSION_MAJOR}")
    set(QT_VERSION_LOWER "(>= ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.0)")
    set(QT_VERSION_UPPER "(<< ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR_PLUS}.0)")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${QT_MEDIA_PACKAGE} ${QT_VERSION_LOWER}, ${QT_MEDIA_PACKAGE} ${QT_VERSION_UPPER}")
    if (USE_MUPDF)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libfreetype6, libharfbuzz0b, libjpeg8, libopenjp2-7, libjbig2dec0")
        if (USE_GUMBO)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libgumbo1")
        endif()
        if (USE_MUJS)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libmujs1")
        endif()
    endif()
    if (USE_POPPLER)
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, libpoppler-qt${QT_VERSION_MAJOR}-1 (>= 0.86.1)")
    endif()
    if (USE_QTPDF)
        set(QT_PDF_PACKAGE "libqt${QT_VERSION_MAJOR}pdf${QT_VERSION_MAJOR}")
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, ${QT_PDF_PACKAGE} ${QT_VERSION_LOWER}, ${QT_PDF_PACKAGE} ${QT_VERSION_UPPER}")
    endif()
    if ("${PACKAGE_ARCH}" STREQUAL "x86_64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    else()
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${PACKAGE_ARCH}")
    endif()
    set(CPACK_DEBIAN_PACKAGE_SECTION "science")
    set(CPACK_DEBIAN_PACKAGE_CONFLICTS "${CONFLICTING_PACKAGES}")
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "libqt${QT_VERSION_MAJOR}svg${QT_VERSION_MAJOR}")
    if (("${QT_VERSION_MAJOR}" EQUAL "5") AND (USE_MUPDF OR USE_POPPLER))
        set(CPACK_DEBIAN_PACKAGE_SUGGESTS "libqt5multimedia5-plugins")
    endif()
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE" DESTINATION "${CMAKE_INSTALL_DOCDIR}" RENAME "copyright")
    install(FILES "${PROJECT_SOURCE_DIR}/LICENSE_AGPL" "${PROJECT_SOURCE_DIR}/LICENSE_GPL" DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()

if (CPACK_GENERATOR MATCHES "RPM")
    # configure RPM package (UNTESTED!)
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "${PACKAGE_ARCH}")
    set(CPACK_RPM_PACKAGE_LICENSE "AGPLv3+")
    set(QT_VERSION_LOWER "(>= ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.0)")
    set(QT_VERSION_UPPER "(< ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR_PLUS}.0)")
    set(CPACK_RPM_PACKAGE_REQUIRES "zlib, qt${QT_VERSION_MAJOR}-qtmultimedia ${QT_VERSION_LOWER}, qt${QT_VERSION_MAJOR}-qtmultimedia ${QT_VERSION_UPPER}")
    if (USE_MUPDF)
        # Probably mujs-devel is required for building if USE_MUJS is ON
        set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, freetype, harfbuzz, libjpeg-turbo, openjpeg2, jbig2dec0-libs")
        if (USE_GUMBO)
            set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, gumbo-parser")
        endif()
        # for building mupdf: mupdf-devel
    endif()
    if (USE_POPPLER)
        set(CPACK_RPM_PACKAGE_REQUIRES "${CPACK_RPM_PACKAGE_REQUIRES}, poppler-qt${QT_VERSION_MAJOR}")
    endif()
    set(CPACK_RPM_PACKAGE_CONFLICTS "${CONFLICTING_PACKAGES}")
endif()

include(CPack)
