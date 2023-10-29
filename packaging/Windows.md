# Building and installing in Windows
It is recommended to use MinGW-w64 in MSYS2.

## Building and installing in MSYS2
1. Install [MSYS2](https://www.msys2.org). After the installation, a terminal for the UCRT64 environment should launch. All commands mentioned in the following should be entered in this terminal. Run `pacman -Syu` directly after the installation.
2. Download the recipe file [PKGBUILD\_MSYS2](PKGBUILD_MSYS2). For the latest mainline version, use [PKGBUILD\_MSYS2\_git](PKGBUILD_MSYS2_git) instead. Place this file in the build directory (e.g. an empty directory) and open this directory in the UXRT64 terminal.
3. Install the basic build tools: `pacman -S base-devel mingw-w64-ucrt-x86_64-gcc`
4. Build and install BeamerPresenter using the command `MINGW_ARCH=ucrt64 _use_poppler=OFF _use_mupdf=ON makepkg-mingw -sip PKGBUILD_MSYS2` (change ON and OFF to select MuPDF or Poppler).
5. Test the installation: run `beamerpresenter` in the terminal.


## Portable binaries (experimental)
Bundling all required dependencies in one directory (e.g. on a USB key) is experimental, but possible.
I do not know what level of compatibility with the build system is required to use this portable version.
Feedback (issues, pull requests) is welcome!

For these instructions it is assumed that BeamerPresenter has been installed in MSYS2 using UCRT64 and that a portable version of BeamerPresenter shall be stored in the directory `output` (which can be renamed and moved afterwards).
Run the following commands in a UCRT64 terminal:
```sh
# Make sure the following command outputs "/ucrt64/bin/beamerpresenter.exe".
which beamerpresenter.exe

# Create the output directory.
mkdir -p output/root/{platforms,multimedia,doc}
cp "$(which beamerpresenter.exe)" output/root
ldd "$(which beamerpresenter.exe)" | sed -n 's/^.*=>\s*\(.*\)\s(0x[0-9a-f]\+)$/\1/p' > output/files.list
# Windows platform plugin is required
cp /ucrt64/share/qt6/plugins/platforms/qwindows.dll output/root/platforms/
ldd /ucrt64/share/qt6/plugins/platforms/qwindows.dll | sed -n 's/^.*=>\s*\(.*\)\s(0x[0-9a-f]\+)$/\1/p' >> output/files.list
# FFmpeg plugin can be used for playing media (optional)
cp /ucrt64/share/qt6/plugins/multimedia/ffmpegmediaplugin.dll output/root/multimedia/
ldd /ucrt64/share/qt6/plugins/multimedia/ffmpegmediaplugin.dll | sed -n 's/^.*=>\s*\(.*\)\s(0x[0-9a-f]\+)$/\1/p' >> output/files.list
# Copy files
cp $(sort --unique output/files.list) output/root
rm output/files.list

# Copy files used by BeamerPresenter.
cp /etc/xdg/beamerpresenter/beamerpresenter.conf output
cp /etc/xdg/beamerpresenter/gui.json output/root
cp -r /ucrt64/share/beamerpresenter/icons output/root
cp /ucrt64/share/doc/beamerpresenter/README.html output/root/doc
for LANG in de en_GB; do
  mkdir -p "output/root/locale/$LANG/LC_MESSAGES"
  cp "/ucrt64/share/locale/${LANG}/LC_MESSAGES/beamerpresenter.qm" "output/root/locale/${LANG}/LC_MESSAGES"
done

# Create a script for running BeamerPresenter with command line arguments.
# This makes sure that the configuration file `beamerpresenter.conf` is read.
echo 'root\beamerpresenter.exe -c beamerpresenter.conf -g root\gui.json' > output/beamerpresenter.bat
```

The output directory can now be copied to a USB key for a portable version of BeamerPresenter. It contains the file "beamerpresenter.bat", which is used to launch BeamerPresenter.

Known limitations
* Some icons are missing. To include icons, copy an icon theme directory to `output/root` and specify it as your preferred icon theme in beamerpresenter.conf.
* The style is ugly, it looks like very old versions of Windows.
* This is only tested with Qt 6 and MuPDF. It should work analogously for other configurations.


## Manually building with Microsoft Visual Studio
It is possible to compile BeamerPresenter on Windows using Microsoft Visual Studio and QtCreator, but this requires some manual configuration.
The following options have been tested (but are not regularly tested):
* Qt 6.5.1 and Qt PDF → very limited features
* Qt 6.5.1 and MuPDF 1.20.3 → you need to build MuPDF first

### Building
This roughly describes how I have built BeamerPresenter.
1. Install Qt
    * [download](https://www.qt.io/download-qt-installer) and install Qt
    * required components are the basic installation, the multimedia module (QtMultimedia) and possibly Qt PDF.
    * compatible Qt versions are ≥5.9 for Qt 5 and ≥6.2 for Qt 6 (when using Qt PDF: ≥5.10 or ≥6.3)
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

### Configuration
In Windows, the configuration is stored in the registry (in something like `HKEY_CURRENT_USER/software/beamerpresenter`).
Once BeamerPresenter is running, you can set most of the settings in the settings widget in BeamerPresenter. But when just trying to run the executable for the first time, a wrong GUI configuration file path might lead to an error. In this case you should run BeamerPresenter on the command line:
```sh
path\to\beamerpresenter.exe -g "path\to\gui.json" "path\to\document.pdf"
```
If no path to a pdf document is provided, a file dialog should appear.
Then you can use the settings widget to change the GUI configuration path and any other option you'd like to change (e.g. the memory size).

Icons will by default be searched relative to the directory containing the executable in `share\icons`. If icons for tools and devices are missing, you should copy the `share\icons` directory of the source to this directory, or try to manually set the "icon path" in the registry for BeamerPresenter to something like "C:\path\to\icons".
The icon theme can be adjusted in the registry by setting "icon theme" for a theme name, or "icon theme path" for the full path to the icon theme.
