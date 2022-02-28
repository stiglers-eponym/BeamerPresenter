# Building and packaging
This file describes how the packages included in the release are built.


## Arch or Manjaro
The Arch packages can be built with the following command:
Download the files `PKGBUILD_poppler` and `PKGBUILD_mupdf` from this directory.
The version with poppler as PDF engine and Qt 6 can be build using:
```sh
_qt_version_major=6 makepkg -p PKGBUILD_poppler
```
The packages for Qt 5 and with MuPDF can be built analogously.
You can install the newly created package using (for version 0.2.2):
```sh
sudo pacman -U beamerpresenter-poppler-qt6-0.2.2-1-x86_64.pkg.tar.zst
```
The "mupdf-small" packages are compiled with a custom build of MuPDF with disabled javascript that excludes some fonts (MuPDF compiled with `XCFLAGS+=' -DTOFU -DTOFU_CJK -DTOFU_SIL -DFZ_ENABLE_JS=0'`).


## Ubuntu
Install the build dependencies:
```sh
sudo apt install --no-install-recommends cmake zlib1g-dev qtmultimedia5-dev qttools5-dev libpoppler-qt5-dev libmupdf-dev libfreetype-dev libharfbuzz-dev libjpeg-dev libopenjp2-7-dev libjbig2dec0-dev
```
Only in Ubuntu 21.10 you additionally need `libmujs-dev`.
In Ubuntu 22.04 you additionally need `libgumbo-dev`.

For version 0.2.2 the source can be downloaded [here](https://github.com/stiglers-eponym/BeamerPresenter/archive/refs/tags/v0.2.2.tar.gz).
Check and unpack the download:
```sh
sha256sum -c - <<< "6a85e9fc6ab7b6e973c85f3146f370d8d48e6c995c4f4e2c712a8e73f8490707 v0.2.2.tar.gz"
tar -xvf v0.2.2.tar.gz
cd BeamerPresenter-0.2.2
```

Now configure the package using CMake. This requires the configuration of the Qt version (major and minor version), and the PDF engine (Poppler or MuPDF). The Qt minor version is only needed for version checking of dependencies.
For building BeamerPresenter with poppler in Ubuntu 20.04 with Qt 5.12 use:
```sh
mkdir -p build_dir
cmake \
    -B build_dir \
    -DCMAKE_BUILD_TYPE='Release' \
    -DGIT_VERSION=OFF \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DUSE_MUJS=OFF \
    -DUSE_GUMBO=OFF \
    -DUSE_TRANSLATIONS=ON \
    -DQT_VERSION_MAJOR=5 \
    -DQT_VERSION_MINOR=12 \
    -DINSTALL_LICENSE=OFF \
    -DCPACK_GENERATOR='DEB;' \
    -DCMAKE_INSTALL_PREFIX='/usr' \
    -DCMAKE_INSTALL_SYSCONFDIR='/etc'
```
* to use MuPDF instead of Poppler: set `-DUSE_POPPLER=OFF` and `-DUSE_MUPDF=ON`
* in Ubuntu 21.10: set `-DQT_VERSION_MINOR=15` and `-DUSE_MUJS=ON` (only relevant when using MuPDF)
* in Ubuntu 22.04 with Qt 5 (untested): set `-DQT_VERSION_MINOR=15` and `-DUSE_GUMBO=ON` (only relevant when using MuPDF)
* in Ubuntu 22.04 with Qt 6 (untested): set `-DQT_VERSION_MAJOR=6`, `-DQT_VERSION_MINOR=2` and `-DUSE_GUMBO=ON` (only relevant when using MuPDF)

Here the build directory was set to `build_dir`, which you can replace by any other empty directory.
We disable `INSTALL_LICENSE` because cpack uses a separate function for installing the license following the conventions of debian packages.

Now build and create the package:
```sh
cmake --build build_dir
cpack --config build_dir/CPackConfig.cmake
```

Now you can install the package:
```sh
sudo apt install ./beamerpresenter-poppler-0.2.2-qt5.12-x86_64.deb
```


## Flatpak
The flatpak package is built using github actions. The build can be reproduced locally using the file `io.github.beamerpresenter.yml`.
