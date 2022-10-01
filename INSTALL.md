# Installing BeamerPresenter
There exist different flavors of BeamerPresenter:
You can choose the PDF engine (Poppler or MuPDF) and the major Qt version (5 or 6), see [below](#choosing-mupdf-or-poppler).

The [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) come with packages for Arch/Manjaro, Ubuntu 20.04, Ubuntu 21.10 and flatpak.
The simplest way to install BeamerPresenter (besides the AUR) is to directly install these packages.
For example, the commands for installing BeamerPresenter with Poppler as PDF engine and Qt 5 after downloading the corresponding file are:
```sh
# Ubuntu 20.04:
sudo apt install ./beamerpresenter-poppler-0.2.3-qt5-focal-x86_64.deb
# Ubuntu 22.04:
sudo apt install ./beamerpresenter-poppler-0.2.3-qt5-jammy-x86_64.deb
# Arch/Manjaro
sudo pacman -U beamerpresenter-poppler-qt5-0.2.3-1-x86_64.pkg.tar.zst
# Flatpak
flatpak install org.kde.Platform/x86_64/5.15-21.08 # can be skipped if already installed
flatpak install beamerpresenter.flatpak
```
The build process for these packages is explained [here](https://github.com/stiglers-eponym/BeamerPresenter/tree/main/packaging).
Verify the signature of the checksums in `SHA256SUMS`:
```sh
gpg --keyserver hkps://keyserver.ubuntu.com --recv-keys DD11316A0D8E585F
gpg --verify SHA256SUMS.sig SHA256SUMS
```

In Arch Linux and Manjaro you can install one of the AUR packages [beamerpresenter](https://aur.archlinux.org/packages/beamerpresenter) and [beamerpresenter-git](https://aur.archlinux.org/packages/beamerpresenter-git).
Note that in these packages by default MuPDF is selected as PDF engine.

There exists a package for [Nix](https://nixos.org), which can also be an option for people using macOS. This package can be installed with
```sh
nix-env -iA nixos.beamerpresenter    # on NixOS
nix-env -iA nixpkgs.beamerpresenter  # on non-NixOS
```


## Choosing the PDF engine
When installing BeamerPresenter you need to choose a PDF engine from MuPDF, Poppler, and Qt PDF.
Enabling multiple PDF engines at compile time is also possible with some limitations (see below).

* Qt PDF only provides a small subset of the features available with Poppler and MuPDF. Missing features include links, page labels, document outline, slide transitions, animations, videos, and sounds.
* Compiling with Qt PDF can be simpler if Qt PDF is available on you system, because you do not need any external libraries besides Qt. Qt PDF is only available in Qt ≥ 5.10 (for Qt 5) or Qt ≥ 6.3 (for Qt 6), but the way how it is included here is probably only compatible with Qt ≥ 5.14 or Qt ≥ 6.3. Even Qt packages with compatible versions do not necessarily include Qt PDF.
* MuPDF produces a much larger package size compared to Poppler: 37MB instead of 1.3MB in Arch Linux in default configuration
    * MuPDF is statically linked
    * MuPDF contains about 30MB of fonts that end up in the executable by default
    * Most built-in fonts can be excluded from MuPDF, shrinking the executable from 37MB to 6.5MB (or even smaller: compile MuPDF with `XCFLAGS+=' -DTOFU -DTOFU_CJK -DTOFU_SIL -DFZ_ENABLE_JS=0'`).
* My impression is that in most cases MuPDF produces better-looking slides than Poppler. But this may depend on the presentation, the fonts, the resolution, ...
* Enabling both Poppler and MuPDF at compile time is not recommended, because it can lead to program crashes when using Poppler for some documents.
* Some features are only supported by Poppler and not by MuPDF. These features include most link types like action links and sound links. For example, the command `\sound{title}{filename}` in LaTeX beamer's multimedia package will only work with Poppler (workaround for MuPDF: use `\movie` instead of `\sound`).
* Integrating MuPDF in BeamerPresenter requires much more code than integrating Poppler or Qt PDF, which might also lead to more bugs.


## General requirements
Building is tested in Arch Linux, Manjaro, Xubuntu 20.04, and Kubuntu 22.04.
Older versions of Ubuntu are only compatible with [version 0.1](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) of BeamerPresenter.

In order to build BeamerPresenter you need to have CMake, zlib and Qt 5/6 including the multimedia module and the linguist tools (only for translations).
In Qt 5 versions since 5.12 are tested, but other versions starting from 5.9 should also be supported. For installation in Qt 6 you need at least version 6.2.
Additionally you need either the Qt 5/6 bindings of Poppler or the MuPDF libraries (which may also require some libraries).

### Dependencies in Ubuntu
* `cmake` (only for building)
    * cmake requires a compiler (e.g. `g++`) and a build system (e.g. Unix makefiles or ninja)
* `zlib1g-dev` (after the installation you can remove `zlib1g-dev` and keep only `zlib1g`)
* `qtmultimedia5-dev` (after the installation you can remove `qtmultimedia5-dev` and keep only `libqt5multimedia5` and `libqt5multimediawidgets5`)
    * or when using Qt 6 in Ubuntu ≥22.04: `qt6-multimedia-dev` and keep `libqt6multimediawidgets6` after the installation
* `qttools5-dev` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))
    * or when using Qt 6 in Ubuntu ≥22.04: `qt6-tools-dev`, `qt6-tools-dev-tools`, and `qt6-l10n-tools`
* optional: `gstreamer1.0-libav` and `libqt5multimedia5-plugins` (for showing videos)
* optional, recommended: `libqt5svg5` (for showing icons)
    * or when using Qt 6 in Ubuntu ≥22.04: `libqt6svg6`

When compiling with Poppler:
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

### Dependencies in Arch Linux and Manjaro
Replace qt6 with qt5 in all package names if you want to use Qt 5.
* `cmake` (only for building and only in the mainline version)
* `qt6-multimedia`
* since Qt ≥6.4: for building you need both backends `qt6-multimedia-ffmpeg` and `qt6-multimedia-gstreamer`
* `qt6-tools` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))
* `qt6-svg` for showing icons

When compiling with Poppler:
* `poppler-qt5`

When compiling with MuPDF:
* `libmupdf` (only for building, tested versions: 1.16.1 – 1.20.3)
* `jbig2dec`
* `openjpeg2`
* `gumbo-parser`

Optional, for showing videos:
* `gst-libav`
* `gst-plugins-good`


## Manual installation
### Download
Download the sources:
```sh
wget https://github.com/stiglers-eponym/BeamerPresenter/archive/v0.2.3.tar.gz
sha256sum -c - <<< "ed4b76e1c51227b538cab4b736113800a1d5069d2131933d56103082c0eb5468 v0.2.3.tar.gz"
tar -xf v0.2.3.tar.gz
cd BeamerPresenter-0.2.3
```
Alternatively, you can clone the git repository
```sh
git clone --depth 1 --single-branch https://github.com/stiglers-eponym/BeamerPresenter.git
cd BeamerPresenter
```

### Configure
Now may you need to configure libraries and file paths in `CMakeLists.txt`. For Ubuntu and Arch the settings are tested, in other GNU+Linux systems you can try if it works, and other systems will probably require a manual configuration.
Pull requests or issues with build instructions for other systems are welcome!

The command line for configuring the build process look like this:
```sh
cmake \
    -B "build-dir"
    -S .
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DUSE_QTPDF=OFF \
    -DUSE_EXTERNAL_RENDERER=OFF \
    -DUSE_MUJS=OFF \
    -DUSE_MUPDF_THIRD=ON \
    -DUSE_GUMBO=ON \
    -DMUPDF_USE_SYSTEM_LIBS=ON \
    -DGIT_VERSION=ON \
    -DUSE_TRANSLATIONS=ON \
    -DINSTALL_LICENSE=ON \
    -DQT_VERSION_MAJOR=6 \
    -DQT_VERSION_MINOR=2 \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_SYSCONFDIR=/etc
```
The options `-B` and `-S` set the build and source directory, respectively. The other options define (with recommended values indicated):

| Option | Value | Explanation |
|--------|-------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Release or Debug |
| `USE_POPPLER` | ON | Include Poppler PDF engine (Poppler library and Qt 5/6 wrapper must be available) |
| `USE_MUPDF` | OFF | Include MuPDF PDF engine (MuPDF static library and headers must be available) |
| `USE_QTPDF` | OFF | Include Qt PDF engine. Note: Only the current git version of BeamerPresenter is compatible with Qt PDF in Qt ≥6.4. |
| `USE_EXTERNAL_RENDERER` | OFF | Include option to use an external program for rendering PDF pages to images. |
| `MUPDF_USE_SYSTEM_LIBS` | ON | MuPDF uses shared system libraries (default in common Linux distributions, disable if you compiled MuPDF from source with standard settings) |
| `USE_MUJS` | OFF | link to MuJS, set ON in Ubuntu ≥21.10 |
| `USE_MUPDF_THIRD` | ON | set OFF when libmupdf-third is not available (for Ubuntu 21.10) |
| `USE_GUMBO` | ON | set ON when using MuPDF >= 1.18 with shared system libraries |
| `GIT_VERSION` | ON | Include git commit count in version string |
| `USE_TRANSLATIONS` | ON | include translations (currently only German), disable if it causes errors |
| `SUPPRESS_MUPDF_WARNINGS` | OFF | Suppress warnings of MuPDF while loading a document (only Unix-like systems) |
| `INSTALL_LICENSE` | ON | Copy the license to /usr/share/licenses/beamerpresenter/LICENSE |
| `QT_VERSION_MAJOR` | 6 | Qt major version, must be set manually! Valid values are "5" and "6". |
| `QT_VERSION_MINOR` | 2 | only relevant for packaging (dependency version checking) |
| `CMAKE_INSTALL_PREFIX` | /usr | Install prefix. If not specified this will be /usr/local in Linux |
| `CMAKE_INSTALL_SYSCONFDIR` | /etc | System config directory. |

### Build and install
After configuring with CMake you can build the project (hint: add ` -j 4` for compiling with 4 CPU cores)
```sh
cmake --build build-dir
```
Then install the package. For packaging the environment variable `$DESTDIR` may be helpful.
```sh
cmake --install build-dir
```

## Windows
It is possible to compile BeamerPresenter with MuPDF on Windows, but it requires a manual configuration.
Compiling only with Qt PDF (without Poppler and MuPDF) is probably simpler, but results in a very limited set of features.

### Summary
* Qt is available for MinGW and for MS Visual Studio.
* MuPDF provides a project file that can be compiled with MS Visual Studio.
* Poppler is available in cygwin. It is possible to build BeamerPresenter with cygwin, MinGW and Poppler. It is probably also possible to build Poppler in Windows, but I gave up at some point.
* Another option is the Windows subsystem for Linux.

### Building
* [download](https://www.qt.io/download-qt-installer) and install Qt
    * required components are the basic installation and the multimedia module (QtMultimedia)
    * compatible Qt versions are >=5.9 for Qt 5 and >=6.2 for Qt 6
* [download the MuPDF source code](https://www.mupdf.com/releases/index.html)
* the MuPDF source code includes a file `platform/win32/mupdf.vcxproj` which can be built with MS Visual Studio
* alternatively, one can try to build MuPDF with MinGW, but I gave up on that.
* the library paths for MuPDF and zlib need to be configured manually in CMake:

Configure the project with CMake (here an example using MuPDF):
```sh
cmake \
    -B "build-dir" \
    -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POPPLER=OFF \
    -DUSE_MUPDF=ON \
    -DUSE_QTPDF=OFF \
    -DUSE_EXTERNAL_RENDERER=OFF \
    -DGIT_VERSION=OFF \
    -DGENERATE_MANPAGES=OFF \
    -DGUI_CONFIG_PATH="config" \
    -DUSE_TRANSLATIONS=ON \
    -DQT_VERSION_MAJOR=6 \
    -DMUPDF_USE_SYSTEM_LIBS=OFF \
    -DMUPDF_INCLUDE_DIR="path/to/mupdf/include" \
    -DMUPDF_LIB_PATH="path/to/libmupdf.lib" \
    -DMUPDF_THIRD_LIB_PATH="path/to/libthirdparty.lib" \
    -DZLIB_INCLUDE_DIR="path/to/zlib/include" \
    -DZLIB_LIBRARY="path/to/zlib.lib"
```
The new options (compared to the table above) are:

| Option | Value | Explanation (bold means you must provide a value) |
|--------|-------|---------------------------------------------------|
| `GENERATE_MANPAGES` | OFF | disable to exclude man pages from installation |
| `GUI_CONFIG_PATH` | "config" | default path for configuration files relative to program data directory |
| `MUPDF_USE_SYSTEM_LIBS` | OFF | Disable when using MuPDF on Windows (except if you know what you are doing) |
| `MUPDF_INCLUDE_DIR` | "" | **path to MuPDF include directory** |
| `MUPDF_LIB_PATH` | "" | **path to libmupdf.lib** |
| `MUPDF_THIRD_LIB_PATH` | "" | **path to libthirdparty.lib** |
| `ZLIB_INCLUDE_DIR` | "" | **path to zlib include directory** |
| `ZLIB_LIBRARY` | "" | **path to zlib.lib** |

Now build the project:
```sh
cmake --build build-dir
```
If this was successful, you can find the executable in "build-dir\src\beamerpresenter.exe".
But note that this executable depends on shared libraries (Qt and (maybe) zlib).

### Installing
not yet tested

### Configuration
The configuration in Windows does not work like on \*NIX systems. The configuration is stored in the Windows registry (in something like `HKEY_CURRENT_USER/software/beamerpresenter`).
Once BeamerPresenter is running, you can set most of the settings in the settings widget in BeamerPresenter. But when just trying to run the executable, this will probably result in an error because the GUI configuration file path is invalid. You should therefore run BeamerPresenter on the command line:
```sh
path\to\beamerpresenter.exe -g "path\to\gui.json" "path\to\document.pdf"
```
If no path to a pdf document is provided, a file dialog should appear.
Then you should use the settings widget to change the GUI configuration path and any other option you'd like to change (e.g. the memory size).

Probably the icons will then still not be available. To enable the icons you need to manually set "icon path" in the registry for BeamerPresenter to something like "C:\path\to\BeamerPresenter\share\icons".
Also the icon theme can be adjusted in the registry. You can try to set a registry entry for "icon theme" to the theme name and hope that Qt finds the theme. Alternatively, you can provide the full path to the icon theme by setting "icon theme path" in the registry.
