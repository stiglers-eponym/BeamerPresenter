# Installing BeamerPresenter
There exist different flavors of BeamerPresenter:
You can choose the PDF engine (Poppler, MuPDF, Qt PDF) and the major Qt version (5 or 6), see [below](#choosing-mupdf-or-poppler).
It is recommended to use Qt 6 (if available) because of noticable improvements when including videos in a presentation.

BeamerPresenter can be found in the official [Nix repositories](https://search.nixos.org/packages?channel=unstable&type=packages&query=BeamerPresenter) and in the [AUR](https://aur.archlinux.org/packages/beamerpresenter) (also as a [mainline version](https://aur.archlinux.org/packages/beamerpresenter-git)).
The [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) include packages for Arch/Manjaro/Endeavour, Ubuntu (24.04, 22.04 and 20.04), and flatpak.
These packages can be installed as shown in the following example, which uses Poppler as PDF engine (after downloading the corresponding file):
```sh
# Ubuntu 24.04:
sudo apt install ./beamerpresenter-poppler-0.2.4-qt6-noble-x86_64.deb
# Ubuntu 22.04:
sudo apt install ./beamerpresenter-poppler-0.2.4-qt5-jammy-x86_64.deb
# Ubuntu 20.04:
sudo apt install ./beamerpresenter-poppler-0.2.4-qt5-focal-x86_64.deb
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
* It is possible to use an external program for rendering slides. However, you still need to provide a PDF engine.


## Requirements
Building is currently tested in Arch Linux, Xubuntu 20.04, Kubuntu 24.04 and 22.04, Fedora 38, and MinGW-w64 in MSYS2 (Windows).

In order to compile BeamerPresenter you need to have CMake, zlib and Qt 5/6 including the multimedia and SVG modules installed.
For translations you also need the linguist tools.
Additionally, you need either the Qt bindings of Poppler, or the MuPDF libraries (which may require other libraries), or Qt PDF.
Qt versions since 5.12 (for Qt 5) or 6.2 (Qt 6) are supported.

### Dependencies in Ubuntu
* `cmake` (only for building)
    * cmake requires a compiler (e.g. `g++`) and a build system (e.g. Unix makefiles or ninja)
* `zlib1g-dev`
* `qt6-multimedia-dev` (Qt 6) or qtmultimedia5-dev (Qt 5)
* `libqt6svg6-dev` (Qt 6) or `libqt5svg5-dev` (Qt 5)
* for translations (only for building, you can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))
    * Qt 6: `qt6-tools-dev`, `qt6-tools-dev-tools`, and `qt6-l10n-tools`
    * Qt 5: `qttools5-dev`
* optional, only for Qt 5: `gstreamer1.0-libav` and `libqt5multimedia5-plugins` (for showing videos)

When compiling with Poppler:
* Qt 5: `libpoppler-qt5-dev`, version 0.86.1 or later
* Qt 6: `libpoppler-qt6-dev`

When compiling with MuPDF:
* `libmupdf-dev` (only for building)
* `libfreetype-dev`
* `libharfbuzz-dev`
* `libjpeg-dev`
* `libopenjp2-7-dev`
* `libjbig2dec0-dev`
* only Ubuntu ≥21.10: `libmujs-dev`
* only Ubuntu ≥22.04: `libgumbo-dev`

When compiling with Qt PDF:
* `qtpdf5-dev` (Qt 5) or `qtpdf6-dev` (Qt 6)

### Dependencies in Arch Linux/Manjaro/Endeavour
Replace qt6 with qt5 in all package names if you want to use Qt 5.
* `cmake` (only for building and only in the mainline version)
* `qt6-multimedia`
    * since Qt ≥6.4 the backend `qt6-multimedia-ffmpeg` is recommended.
* `qt6-svg`
* `qt6-tools` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [CMake command](#configure))

When compiling with Poppler:
* `poppler-qt6`

When compiling with MuPDF:
* `libmupdf` (tested versions: 1.16.1 – 1.24.6)
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

The command line for configuring the build process with some basic options looks like this:
```sh
cmake \
    -B "build-dir" \
    -S . \
    -DCMAKE_BUILD_TYPE=Release \
    -DQT_VERSION_MAJOR=6 \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DGIT_VERSION=ON
```
The options `-B` and `-S` set the build and source directory, respectively.
Other options are added in the form `-DOPTION=VALUE` as listed below.

### CMake options
#### Features and libraries
| Option | Value | Explanation |
| ------ | ----- | ----------- |
| `QT_VERSION_MAJOR` | 6 | Qt major version, must be set manually! Valid values are "5" and "6". |
| `CMAKE_BUILD_TYPE` | Release | set to "Debug" to include debugging information |
| `USE_POPPLER` | ON | include Poppler PDF engine (Poppler library and Qt 5/6 headers must be available) |
| `USE_MUPDF` | OFF | include MuPDF PDF engine (MuPDF static library and headers must be available) |
| `USE_QTPDF` | OFF | include Qt PDF engine, which offers limited features. |
| `USE_EXTERNAL_RENDERER` | OFF | include option to use an external program for rendering PDF pages to images. |
| `USE_WEBCAMS` | ON | allow using webcams as video source. |
| `USE_TRANSLATIONS` | ON | include translations (currently only German) |
| `GIT_VERSION` | ON | include git commit count in version string |
| `SUPPRESS_MUPDF_WARNINGS` | OFF | suppress warnings of MuPDF while loading a document (only Unix-like systems) |

#### Linker options and technical details
| Option | Value | Explanation |
| ------ | ----- | ----------- |
| `MUPDF_USE_SYSTEM_LIBS` | ON | MuPDF uses system libraries (default in common Linux distributions) |
| `LINK_MUJS` | OFF | link to MuJS, set ON in Ubuntu ≥21.10 |
| `LINK_MUPDF_THIRD` | ON | set OFF when libmupdf-third is not available (Ubuntu 21.10 and Arch Linux) |
| `LINK_GUMBO` | ON | set ON when using MuPDF >= 1.18 with shared system libraries |
| `LINK_TESSERACT` | OFF | set ON when using MuPDF in Fedora |

#### Options only affecting the installation
| Option | Value | Explanation |
| ------ | ----- | ----------- |
| `INSTALL_LICENSE` | ON | copy the license to /usr/share/licenses/beamerpresenter/LICENSE |
| `QT_VERSION_MINOR` | 6 | only relevant for packaging (dependency version checking) |
| `CMAKE_INSTALL_PREFIX` | /usr | install prefix. If not specified, this will be /usr/local in Linux |
| `CMAKE_INSTALL_SYSCONFDIR` | /etc | system configuration directory |
| `GENERATE_MANPAGES` | ON | Generate man pages and include them in installation |
| `UBUNTU_VERSION` | 24.04 | Adjust package dependencies to given Ubuntu version when building debian package |

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
In Windows, it is recommended to use MinGW-w64 in MSYS2.
For more details and other options, see [packaging/Windows.md](packaging/Windows.md).

MinGW-w64 can be obtained in different ways. I have only tested MSYS2 using the native C runtime in Windows.

1. Install [MSYS2](https://www.msys2.org). After the installation, a terminal for the UCRT64 environment should launch. All commands mentioned in the following should be entered in this terminal. Run `pacman -Syu` directly after the installation.
2. Download the recipe file [packaging/PKGBUILD\_MSYS2](packaging/PKGBUILD_MSYS2). Place this file in the build directory (e.g. an empty directory) and open this directory in the UXRT64 terminal (program "MSYS2 UCRT64" in Windows).
3. Install the basic build tools: `pacman -S base-devel mingw-w64-ucrt-x86_64-gcc`
4. Build and install BeamerPresenter using the command `MINGW_ARCH=ucrt64 makepkg-mingw -sip PKGBUILD_MSYS2`. By default, this uses MuPDF as PDF engine. You can use Poppler instead with the command `MINGW_ARCH=ucrt64 _use_poppler=ON _use_mupdf=OFF makepkg-mingw -sip PKGBUILD_MSYS2`. This should automatically install dependencies and eventually install the package.
5. Test the installation: run `beamerpresenter` in the terminal.
