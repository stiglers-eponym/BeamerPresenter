# Installing BeamerPresenter
There exist different flavors of BeamerPresenter:
You can choose the PDF engine (Poppler or MuPDF) and the major Qt version (5 or 6), see [below](#choosing_mupdf_or_poppler).

In Arch Linux and Manjaro you can install one of the AUR packages [beamerpresenter](https://aur.archlinux.org/packages/beamerpresenter) and [beamerpresenter-git](https://aur.archlinux.org/packages/beamerpresenter-git).
Note that in these packages by default MuPDF is selected as PDF engine.

There exists also a package for Nix (thanks to the maintainer of that one!) which can be installed with
```sh
nix-env -iA nixos.beamerpresenter    # on NixOS
nix-env -iA nixpkgs.beamerpresenter  # on non-NixOS
```

The [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) come with packages for Arch/Manjaro, Ubuntu 20.04, Ubuntu 21.10 and flatpak.
The simplest way to install BeamerPresenter (besides the AUR) is to directly install these packages.
For example, the commands for installing BeamerPresenter with poppler as PDF engine and Qt 5 after downloading the corresponding file are:
```sh
# Ubuntu 20.04:
sudo apt install ./beamerpresenter-poppler-0.2.2-beta2-qt5.12-x86_64.deb
# Ubuntu 21.10:
sudo apt install ./beamerpresenter-poppler-0.2.2-beta2-qt5.15-x86_64.deb
# Arch/Manjaro
sudo pacman -U beamerpresenter-poppler-qt5-0.2.2_beta2-1-x86_64.pkg.tar.zst
# Flatpak
flatpak install org.kde.Platform/x86_64/5.15-21.08 # can be skipped if already installed
flatpak install beamerpresenter.flatpak
```
The build process for these packages is explained [here](https://github.com/stiglers-eponym/BeamerPresenter/tree/main/packaging).


## Choosing MuPDF or Poppler
When installing BeamerPresenter you need to choose either MuPDF or Poppler as PDF engine. Here is my personal opinion that might help you with the decision.

* MuPDF produces a much larger package size: 37MB instead of 1.3MB in Arch Linux.
    * MuPDF is statically linked and contains about 30MB of fonts that end up in the executable
    * When using MuPDF the size of the executable can be reduced from 37MB to 7MB by compiling MuPDF without extra fonts.
* MuPDF may have better performance.
* My impression is that in most cases MuPDF produces slightly better-looking slides than Poppler. But this may depend on the presentation, the fonts, the resolution, ...
* Enabling both PDF engines is not recommended, because it can lead to program crashes when using Poppler for some documents.
* Some features are only supported by Poppler and not by MuPDF. These features include most link types like action links and sound links. For example, the command `\sound{title}{filename}` in LaTeX beamer's multimedia package will only work with Poppler (workaround: use `\movie` instead of `\sound`).
* Integrating MuPDF in BeamerPresenter requires much more code than integrating Poppler, which might also lead to more bugs.


## General requirements
Building is tested in Arch Linux, Manjaro, xubuntu 20.04, and xubuntu 21.10.
Older versions of ubuntu are only compatible with [version 0.1](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) of BeamerPresenter.

In order to build BeamerPresenter you need to have cmake, zlib and Qt 5/6 including the multimedia module and the linguist tools.
In Qt 5 versions since 5.12 are tested, but other versions starting from 5.9 should also be supported. For installation in Qt 6 you need at least version 6.2.
Additionally you need either the Qt 5/6 bindings of poppler or the MuPDF static library (which also requires some libraries).

### Dependencies in Ubuntu
* `cmake` (only for building and only in the mainline version)
* `zlib1g-dev` (after the installation you can remove `zlib1g-dev` and keep only `zlib1g`)
* `qtmultimedia5-dev` (after the installation you can remove `qtmultimedia5-dev` and keep only `libqt5multimedia5` and `libqt5multimediawidgets5`)
* `qttools5-dev` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [cmake command](#build))
* optional: `libqt5multimedia5-plugins` (for showing videos)
* optional, recommended: `libqt5svg5` (for showing tool icons)

For poppler (optional, see [below](#build)):
* `libpoppler-qt5-dev`: version 0.86.1 is tested, versions below 0.70 are explicitly not supported. (after the installation you can remove `libpoppler-qt5-dev` and keep only `libpoppler-qt5-1`

For MuPDF (optional, see [below](#build)):
* `libmupdf-dev` (only for building): Tested versions: 1.16.1 - 1.19.0.
* `libfreetype-dev` (after the installation you can remove `libfreetype-dev` and keep only `libfreetype6`)
* `libharfbuzz-dev` (after the installation you can remove `libharfbuzz-dev` and keep only `libharfbuzz0b`)
* `libjpeg-dev` (after the installation you can remove `libjpeg-dev` and keep only `libjpeg8`)
* `libopenjp2-7-dev` (after the installation you can remove `libopenjp2-7-dev` and keep only `libopenjp2-7`)
* `libjbig2dec0-dev` (after the installation you can remove `libjbig2dec0-dev` and keep only `libjbig2dec0`)
* Only in Ubuntu 21.10: `libmujs-dev` (after the installation you can remove `libmujs-dev` and keep only `libmujs1`)

### Dependencies in Arch Linux and Manjaro
Replace qt5 by qt6 in all package names if you want to use Qt 6.
* `cmake` (only for building and only in the mainline version)
* `qt5-multimedia` (depends on `qt5-base`, which is also required)
* `qt5-tools` (only for building and only when creating translations. You can disable translations with `-DUSE_TRANSLATIONS=OFF` in the [cmake command](#build))
* optional: `qt5-svg` for showing icons

For poppler (optional, see [below](#build)):
* `poppler-qt5`

For MuPDF (optional, see [below](#build)):
* `libmupdf` (only for building, tested versions: 1.16.1 - 1.19.0)
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
wget https://github.com/stiglers-eponym/BeamerPresenter/archive/v0.2.2_beta2.tar.gz
sha256sum -c - <<< "cf8904563e9b1a9a1ed0cecb65e27ae1ba99e173d9b69cf1e53275294abb9811 v0.2.2_beta2.tar.gz"
tar -xf v0.2.2_beta2.tar.gz
cd BeamerPresenter-0.2.2_beta2
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
    -B "build-dir" \ # build directory
    -S . \ # source directory
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DUSE_MUJS=OFF \ # set ON when libmupdf-third is not available
    -DUSE_GUMBO=ON \ # set ON when using MuPDF >= 1.18
    -DGIT_VERSION=ON \
    -DUSE_TRANSLATIONS=ON \
    -DQT_VERSION_MAJOR=6 \ # must be set manually!
    -DQT_VERSION_MINOR=2 \ # only relevant for packaging
    -DCREATE_SHARED_LIBRARIES=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_SYSCONFDIR=/etc
```
The most important options here are:
* `-DCMAKE_BUILD_TYPE`: Enable debugging by setting this to `Debug` instead of `Release`.
* `-DUSE_POPPLER`: enable PDF engine Poppler. The Poppler library and its Qt 5/6 wrapper must be available.
* `-DUSE_MUPDF`: enable PDF engine MuPDF. The MuPDF static library and headers as well as the other dependencies listed above must be available.
* `-DUSE_MUJS`: set this to "ON" in Ubuntu 21.10 (only relevant when using MuPDF)
* `-DUSE_GUMBO`: can be set to "OFF" when using MuPDF < 1.18 (only relevant when using MuPDF)
* `-DUSE_TRANSLATIONS`: Include translations (currently only in German) when compiling and installing.
* `-DQT_VERSION_MAJOR`: Qt version, must be set manually! Valid values are `5` and `6`.
* `-DCMAKE_INSTALL_PREFIX`: path for package installation.
* `-DCMAKE_INSTALL_SYSCONFDIR`: path for installation of package configuration.
* `-DGIT_VERSION`: Include git commit count in the app version number.

### Build and install
After configuring with cmake you can build the project (hint: add ` -j 4` for compiling with 4 CPU cores)
```sh
cmake --build build-dir
```
Then install the package. For packaging the environment variable `$DESTDIR` may be helpful.
```sh
cmake --install build-dir
```
