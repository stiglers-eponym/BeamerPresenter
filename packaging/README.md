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
You can install the newly created package using
```sh
sudo pacman -U beamerpresenter-poppler-qt6-0.2.2_beta-1-x86_64.pkg.tar.zst
```


## Ubuntu
Install the build dependencies:
```sh
sudo apt install --no-install-recommended cmake zlib1g-dev qtmultimedia5-dev qttools5-dev libpoppler-qt5-dev libmupdf-dev libfreetype-dev libharfbuzz-dev libjpeg-dev libopenjp2-7-dev libjbig2dec0-dev libgumbo-dev
```

For version 0.2.2-beta the source can be downloaded [here](https://github.com/stiglers-eponym/BeamerPresenter/archive/refs/tags/v0.2.2_beta.tar.gz).
Check and unpack the download:
```sh
sha256sum -c - <<< "15e4b07cb22256ea199eab40abf0fd6c780248992278f195c0bc0cb17e2508ce v0.2.2_beta.tar.gz"
tar -xvf v0.2.2_beta.tar.gz
cd BeamerPresenter-0.2.2_beta
```

Now configure the package using cmake. This requires the configuration of the Qt version (major and minor version), and the PDF engine (Poppler or MuPDF). The Qt minor version is only needed for version checking of dependencies.
For building BeamerPresenter with poppler in Ubuntu 20.04 with Qt 5.12 use:
```sh
cmake \
    -B build_dir \
    -DCMAKE_BUILD_TYPE='Release' \
    -DGIT_VERSION=OFF \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DUSE_TRANSLATIONS=ON \
    -DQT_VERSION_MAJOR=5 \
    -DQT_VERSION_MINOR=12 \
    -DCREATE_SHARED_LIBRARIES=OFF \
    -DCPACK_GENERATOR="DEB;" \
    -DCMAKE_INSTALL_PREFIX='/usr' \
    -DCMAKE_INSTALL_SYSCONFDIR='/etc'
```
Here the build directory was set to `build_dir`, but that could also be any other directory.
The MuPDF version can be built by setting `-DUSE_POPPLER=OFF` and `-DUSE_MUPDF=ON` in the above command.

Now build and create the package:
```sh
mkdir -p build_dir
cmake --build build_dir # this is the slow part, maybe add -j4 to speed it up
cpack --config build_dir/CPackConfig.cmake
```

Now you can install the package:
```sh
sudo dpkg -i beamerpresenter-poppler-0.2.2-beta-qt5.12-x86_64.deb
```


## Flatpak
The flatpak package is built using github actions. The build can be reproduced locally using the file `io.github.beamerpresenter.yml`.
