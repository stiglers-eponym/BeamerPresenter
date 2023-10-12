# Installing BeamerPresenter
There exist different flavors of BeamerPresenter:
You can choose the PDF engine (Poppler, MuPDF, Qt PDF) and the major Qt version (5 or 6), see [below](#choosing-mupdf-or-poppler).
It is recommended to use Qt 6 (if available) because of noticable improvements when including videos in a presentation.

BeamerPresenter can be found in the official [Nix repositories](https://search.nixos.org/packages?channel=unstable&type=packages&query=BeamerPresenter) and in the [AUR](https://aur.archlinux.org/packages/beamerpresenter) (also as a [mainline version](https://aur.archlinux.org/packages/beamerpresenter-git)).
The [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) include packages for Arch/Manjaro/Endeavour, Ubuntu 22.04, Ubuntu 20.04, and flatpak.
These packages can be installed as shown in the following example, which uses Poppler as PDF engine (after downloading the corresponding file):
```sh
# Ubuntu 20.04:
sudo apt install ./beamerpresenter-poppler-0.2.4-qt5-focal-x86_64.deb
# Ubuntu 22.04:
sudo apt install ./beamerpresenter-poppler-0.2.4-qt5-jammy-x86_64.deb
# Arch/Manjaro/Endeavour
sudo pacman -U beamerpresenter-poppler-qt6-0.2.4-1-x86_64.pkg.tar.zst
# Flatpak
flatpak install org.kde.Platform/x86_64/6.4 # can be skipped if already installed
flatpak install BeamerPresenter-Qt6.flatpak
```
Verify the signature of the checksums in `SHA256SUMS`:
```sh
gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys DD11316A0D8E585F
gpg --verify SHA256SUMS.sig SHA256SUMS
```


## Choosing the PDF engine
When installing BeamerPresenter you need to choose a PDF engine from MuPDF, Poppler, and Qt PDF.

* My personal impression is that in most cases MuPDF produces better-looking slides than Poppler.
* Some features are only supported by Poppler and not by MuPDF. For example, the command `\sound{title}{filename}` in LaTeX beamer's multimedia package will only work with Poppler (workaround for MuPDF: use `\movie` instead of `\sound`).
* Enabling both Poppler and MuPDF at compile time is not recommended. For some documents this leads to a crash of BeamerPresenter.
* Qt PDF provides very limited features. Only use it as a fallback if MuPDF and Poppler cannot be used. Qt PDF requires a recent version of Qt (≥5.14 or ≥6.3).
* On some platforms, MuPDF produces a larger package size. For some Linux distributions, using MuPDF from official repositories leads to very large packages (>20MB). Building MuPDF manually can significantly reduce the package size (use options like `XCFLAGS+=' -DTOFU -DTOFU_CJK -DTOFU_SIL -DFZ_ENABLE_JS=0'`).


## Requirements
Building is mainly tested in Arch Linux, Xubuntu 20.04, and Kubuntu 22.04. Build instructions also exists for Fedora 38 and MSYS2 (on Windows), but these are not regularly tested.
Older versions of Ubuntu are only compatible with [version 0.1](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) of BeamerPresenter.

In order to compile BeamerPresenter you need to have CMake, zlib and Qt 5/6 including the multimedia and SVG modules installed.
For translations you also need the linguist tools.
Additionally, you need either the Qt bindings of Poppler, or the MuPDF libraries (which may require other libraries).
Qt versions since 5.12 (for Qt 5) or 6.2 (Qt 6) are supported.

### Dependencies in Ubuntu
* `cmake` (only for building)
    * cmake requires a compiler (e.g. `g++`) and a build system (e.g. Unix makefiles or ninja)
* `zlib1g-dev` (after the installation you can remove `zlib1g-dev` and keep only `zlib1g`)
* `qt6-multimedia-dev` (after the installation, you can remove `qt6-multimedia6-dev` and keep only `libqt6multimediawidgets6`)
    * for Qt 5 in Ubuntu 20.04: install `qtmultimedia5-dev`, and keep `libqt5multimedia5` and `libqt5multimediawidgets5` after the installation
* `libqt6svg6-dev` (after the installation, you can remove `libqt6svg6-dev` and keep only `libqt6svg6`)
    * when using Qt 5: `libqt5svg5-dev` and keep `libqt5svg5` after the installation
* `qt6-tools-dev`, `qt6-tools-dev-tools`, and `qt6-l10n-tools` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))
    * for Qt 5: `qttools5-dev`
* optional and only for Qt 5: `gstreamer1.0-libav` and `libqt5multimedia5-plugins` (for showing videos, when using Qt 5)

When compiling with Poppler (only available with Qt 5):
* `libpoppler-qt5-dev`: version 0.86.1 or later. (after the installation you can remove `libpoppler-qt5-dev` and keep only `libpoppler-qt5-1`

When compiling with MuPDF:
* `libmupdf-dev` (only for building)
* `libfreetype-dev` (after the installation you can remove `libfreetype-dev` and keep only `libfreetype6`)
* `libharfbuzz-dev` (after the installation you can remove `libharfbuzz-dev` and keep only `libharfbuzz0b`)
* `libjpeg-dev` (after the installation you can remove `libjpeg-dev` and keep only `libjpeg8`)
* `libopenjp2-7-dev` (after the installation you can remove `libopenjp2-7-dev` and keep only `libopenjp2-7`)
* `libjbig2dec0-dev` (after the installation you can remove `libjbig2dec0-dev` and keep only `libjbig2dec0`)
* only Ubuntu ≥21.10: `libmujs-dev` (after the installation you can remove `libmujs-dev` and keep only `libmujs1`)
* only Ubuntu ≥22.04: `libgumbo-dev` (after the installation you can remove `libgumbo-dev` and keep only `libgumbo1`)

When compiling with Qt PDF (only Qt 5 and Ubuntu >= 21.04):
* `qtpdf5-dev` (after the installation you can remove `qtpdf5-dev` and keep only `libqt5pdf5`)

### Dependencies in Arch Linux/Manjaro/Endeavour
Replace qt6 with qt5 in all package names if you want to use Qt 5.
* `cmake` (only for building and only in the mainline version)
* `qt6-multimedia`
    * since Qt ≥6.4 the backend `qt6-multimedia-ffmpeg` is recommended.
* `qt6-svg`
* `qt6-tools` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))

When compiling with Poppler:
* `poppler-qt5`

When compiling with MuPDF:
* `libmupdf` (only for building, tested versions: 1.16.1 – 1.23.3)
* `jbig2dec`
* `openjpeg2`
* `gumbo-parser`

Optional, for showing videos:
* `gst-libav`
* `gst-plugins-good`

### Dependencies in Fedora
Fedora is the only RPM-based system tested so far. Please open an issue if these instructions seem wrong or outdated!

General build dependencies in Fedora 38:
* `cmake`
* `git` (only when building mainline version)
* `zlib-devel`
* `qt5-qtmultimedia-devel` (`qt6-qtmultimedia-devel` for Qt 6)
* `qt5-qtsvg-devel` (`qt6-qtsvg-devel` for Qt 6)
* `qt5-qttools-devel` (`qt6-qttools-devel` for Qt 6)
* `fedora-packager`

When using poppler:
* `poppler-qt5-devel` (`poppler-qt6-devel` for Qt 6)

When using MuPDF:
* `mupdf-devel`
* `freetype-devel`
* `harfbuzz-devel`
* `libjpeg-turbo-devel`
* `openjpeg2-devel`
* `jbig2dec-devel`
* `gumbo-parser-devel`
* `tesseract-devel` (use the option `LINK_TESSERACT=ON`)


## Manual installation
### Download
Download the sources:
```sh
wget https://github.com/stiglers-eponym/BeamerPresenter/archive/v0.2.4.tar.gz
sha256sum -c - <<< "4ccdd747b2c829411de3f33548a125f8e7f16a768e03f56f71bd6b3f27f5bca1 v0.2.4.tar.gz"
tar -xf v0.2.4.tar.gz
cd BeamerPresenter-0.2.4
```
Alternatively, you can clone the git repository
```sh
git clone --depth 1 --single-branch https://github.com/stiglers-eponym/BeamerPresenter.git
cd BeamerPresenter
```

### Configure
Now may you need to configure libraries and file paths in `CMakeLists.txt`. For Ubuntu and Arch the settings are tested, in other GNU+Linux systems you can try if it works, and other systems will probably require a manual configuration.
Please open an issue if you have questions.
Pull requests or issues with build instructions for other systems are welcome!

The command line for configuring the build process looks like this (not all options are required):
```sh
cmake \
    -B "build-dir"
    -S .
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DUSE_QTPDF=OFF \
    -DUSE_EXTERNAL_RENDERER=OFF \
    -DLINK_MUJS=OFF \
    -DLINK_MUPDF_THIRD=ON \
    -DLINK_GUMBO=ON \
    -DLINK_TESSERACT=OFF \
    -DMUPDF_USE_SYSTEM_LIBS=ON \
    -DGIT_VERSION=ON \
    -DUSE_TRANSLATIONS=ON \
    -DINSTALL_LICENSE=ON \
    -DQT_VERSION_MAJOR=6 \
    -DQT_VERSION_MINOR=5 \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_SYSCONFDIR=/etc
```
The options `-B` and `-S` set the build and source directory, respectively. The other options define (with recommended values indicated):

| Option | Value | Explanation |
|--------|-------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Release or Debug |
| `USE_POPPLER` | ON | Include Poppler PDF engine (Poppler library and Qt 5/6 headers must be available) |
| `USE_MUPDF` | OFF | Include MuPDF PDF engine (MuPDF static library and headers must be available) |
| `USE_QTPDF` | OFF | Include Qt PDF engine. |
| `USE_EXTERNAL_RENDERER` | OFF | Include option to use an external program for rendering PDF pages to images. |
| `MUPDF_USE_SYSTEM_LIBS` | ON | MuPDF uses shared system libraries (default in common Linux distributions) |
| `LINK_MUJS` | OFF | link to MuJS, set ON in Ubuntu ≥21.10 |
| `LINK_MUPDF_THIRD` | ON | set OFF when libmupdf-third is not available (only Ubuntu 21.10) |
| `LINK_GUMBO` | ON | set ON when using MuPDF >= 1.18 with shared system libraries |
| `LINK_TESSERACT` | OFF | set ON when using MuPDF in Fedora |
| `GIT_VERSION` | ON | Include git commit count in version string |
| `USE_TRANSLATIONS` | ON | include translations (currently only German) |
| `SUPPRESS_MUPDF_WARNINGS` | OFF | Suppress warnings of MuPDF while loading a document (only Unix-like systems) |
| `INSTALL_LICENSE` | ON | Copy the license to /usr/share/licenses/beamerpresenter/LICENSE |
| `QT_VERSION_MAJOR` | 6 | Qt major version, must be set manually! Valid values are "5" and "6". |
| `QT_VERSION_MINOR` | 5 | only relevant for packaging (dependency version checking) |
| `CMAKE_INSTALL_PREFIX` | /usr | Install prefix. If not specified this will be /usr/local in Linux |
| `CMAKE_INSTALL_SYSCONFDIR` | /etc | System configuration directory. |

### Build and install
After configuring with CMake, you can build the project (add ` -j 4` for compiling with 4 CPU cores)
```sh
cmake --build build-dir
```
Then install the package.
```sh
cmake --install build-dir
```


## Windows
In Windows, it is recommended to use MinGW-w64.
Alternatively, BeamerPresenter can be built manually using Microsoft Visual Studio.

### MinGW-w64 + MSYS2
MinGW-w64 can be obtained in different ways. I have only tested MSYS2 using the native C runtime in Windows.

1. Install [MSYS2](https://www.msys2.org). After the installation, a terminal for the UCRT64 environment should launch. All commands mentioned in the following should be entered in this terminal. Run `pacman -Syu` directly after the installation.
2. Download the recipe file [packaging/PKGBUILD\_MSYS2\_git](packaging/PKGBUILD_MSYS2_git). This file contains the instructions to build BeamerPresenter. Place this file in the build directory (e.g. an empty directory) and enter this directory in the terminal.
3. Install the basic build tools (`pacman -S base-devel mingw-w64-ucrt-x86_64-gcc`).
4. Build using the command `MINGW_ARCH=ucrt64 makepkg-mingw -sp PKGBUILD_MSYS2_git`. By default, this uses Poppler as PDF engine. You can use MuPDF instead with the command `MINGW_ARCH=ucrt64 _use_poppler=OFF _use_mupdf=ON makepkg-mingw -sip PKGBUILD_MSYS2_git`. This should automatically install other dependencies and in the end install the package.
5. Test the installation: run `beamerpresenter` in the terminal.

### Microsoft Visual Studio
It is possible to compile BeamerPresenter on Windows using Microsoft Visual Studio and QtCreator, but this requires some manual configuration.
The following options have been tested (but are not regularly tested):
* Qt 6.5.1 and QtPDF → very limited features
* Qt 6.5.1 and MuPDF 1.20.3 → you need to build MuPDF first

#### Building
This roughly describes how I have built BeamerPresenter.
1. Install Qt
    * [download](https://www.qt.io/download-qt-installer) and install Qt
    * required components are the basic installation, the multimedia module (QtMultimedia) and possibly QtPDF.
    * compatible Qt versions are ≥5.9 for Qt 5 and ≥6.2 for Qt 6 (when using QtPDF: ≥5.10 or ≥6.3)
2. Only when using MuPDF: Build MuPDF
    * [download the MuPDF source code](https://www.mupdf.com/releases/index.html). Versions 1.20.3 and 1.19.1 worked, but for 1.22.1 and 1.21.1 I got a linker error.
    * build libmupdf with MSVC. The MuPDF source code includes a file `platform/win32/mupdf.sln` which allows you to build libmupdf with MSVC
3. Build zlib:
    * [download the zlib source code](https://www.zlib.net)
    * build zlibstat using MSVC: The sources include a file `contrib/vstudio/vc14/zlibvc.sln` which can be opened with MSVC and allows you to build zlibstat.
    * I use static linking (zlibstat.lib instead of zlib.lib) to simplify deployment.
4. Adapt the library files and include paths for zlib and MuPDF (if enabled) in CMake. Recommended options for building with MuPDF are:

| Option | (Example) value | Explanation (bold means you must provide a value) |
|--------|-----------------|---------------------------------------------------|
| `GIT_VERSION` | OFF | disable git commit count in version string |
| `GENERATE_MANPAGES` | OFF | disable to exclude man pages from installation (generating man pages requires gzip) |
| `GUI_CONFIG_PATH` | "config" | default path for configuration files relative to program data directory |
| `MUPDF_USE_SYSTEM_LIBS` | OFF | Disable when using MuPDF on Windows (except if you know what you are doing) |
| `MUPDF_INCLUDE_DIR` | "full/path/to/mupdf-source/include" | **full path to MuPDF include directory** |
| `MUPDF_LIB_PATH` | "full/path/to/mupdf-source/platform/win32/x64/Release/libmupdf.lib" | **full path to libmupdf.lib** |
| `MUPDF_THIRD_LIB_PATH` | "full/path/to/mupdf-source/platform/win32/x64/Release/libthirdparty.lib" | **full path to libthirdparty.lib** |
| `ZLIB_INCLUDE_DIR` | "full/path/to/zlib" | **full path to zlib source directory** |
| `ZLIB_LIBRARY` | "full/path/to/zlib/contrib/vstudio/vc14/x64/ZlibStatRelease/zlibstat.lib" | **full path to zlibstat.lib** |

5. Build the project using CMake.
If this was successful, you can find the executable in "path\to\build\src\beamerpresenter.exe".
But this executable depends on shared libraries (Qt and compiler-dependent).
When using QtCreator and Microsoft Visual Studio, the following example might guide you to deploying the app.

Assume that the current directory is the root of the source code and build-dir contains the build.
We want to bundle all files required for an installation in build-dir\deploy.
```bat
cd build-dir
mkdir deploy
copy build-dir\src\beamerpresenter.exe deploy
cd deploy
REM Set up the build environment for Qt
C:\Qt\6.5.1\msvc2019_64\bin\qtenv2.bat
REM include MSVC variables in build environment
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
REM use Qt script to collect all requried shared libraries
C:\Qt\6.5.1\msvc2019_64\bin\windeploy6.exe beamerpresenter.exe
REM copy required files to deploy
mkdir share\doc
copy ..\share\doc\README.html share\doc
mkdir config
copy ..\config\beamerpresenter.conf config
copy ..\..\config\gui.json config
copy ..\..\share\icons share
```

The generated directory `deploy` now contains the full program. It can probably be distributed to other systems which have the same runtime library installed.

#### Configuration
In Windows, the configuration is stored in the registry (in something like `HKEY_CURRENT_USER/software/beamerpresenter`).
Once BeamerPresenter is running, you can set most of the settings in the settings widget in BeamerPresenter. But when just trying to run the executable for the first time, a wrong GUI configuration file path might lead to an error. In this case you should run BeamerPresenter on the command line:
```sh
path\to\beamerpresenter.exe -g "path\to\gui.json" "path\to\document.pdf"
```
If no path to a pdf document is provided, a file dialog should appear.
Then you can use the settings widget to change the GUI configuration path and any other option you'd like to change (e.g. the memory size).

Icons will by default be searched relative to the directory containing the executable in `share\icons`. If icons for tools and devices are missing, you should copy the `share\icons` directory of the source to this directory, or try to manually set the "icon path" in the registry for BeamerPresenter to something like "C:\path\to\icons".
The icon theme can be adjusted in the registry by setting "icon theme" for a theme name, or "icon theme path" for the full path to the icon theme.
