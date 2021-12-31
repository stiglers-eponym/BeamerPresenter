# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes a
clock, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF and/or poppler.

## Features (selection)
* modular user interface: adapt for your presentation style and technical equipment (number of monitors, extra information for the speaker, input devices, ...)
* compressed cache for fast slide changes
* draw in slides, save drawings in a format compatible with [Xournal++](https://xournalpp.github.io)
* highlighting tools (pointer, torch, magnifier)
* rich text notes for the speaker
* (optionally) show separate presentation file for speaker or use LaTeX-beamer's option to show notes on second screen (split PDF pages into a part for the speaker and a part or the audience)
* timer indicates progress relative to a predefined schedule by it's color
* navigate using document outline, thumbnail slides, page numbers/labels and links
* videos in presentations (currently sound is untested)
* slide transitions


## Screenshots
These screenshots only show one possible way of using BeamerPresenter. The speaker could also see a different presentation (with additional information) or editable notes.

<table border="0px" >
<tr>
<td width=50%>
One possible configuration of the graphical interface shows the previews of the last overlays of the current and next slide to the speaker:
<img srcset=".readme/titleslide-640px.webp 640w, .readme/titleslide-960px.webp 960w" sizes="(max-width: 640px) 640px, 960px" src=".readme/titleslide-960px.webp" width=100% title="Two windows for speaker and audiance showing a title slide.">
The small window in this image is the presentation window that is usually shown on a projector.
</td>
<td>
A timer shows your current progress. When you have a tight schedule you can plan times for some slides and the color of the timer will indicate your progress relative to these planned times.
<img srcset=".readme/timer-640px.webp 640w, .readme/timer-960px.webp 960w" sizes="(max-width: 640px) 640px, 960px" src=".readme/timer-960px.webp" width=100% title="Annotations on a slide explain the timer which is shown on the speaker's screen.">
The annotations on this slide (red text and arrow) were also made directly in BeamerPresenter.
Annotations can be saved in a format that is compatible with <a href="https://xournalpp.github.io">Xournal++</a>.
</td>
</tr>
<tr>
<td>
Different highlighting tool, including magnifier, torch and pointer, can be used to draw attention to parts of your slide.
<img srcset=".readme/magnifier-640px.webp 640w, .readme/magnifier-960px.webp 960w" sizes="(max-width: 640px) 640px, 960px" src=".readme/magnifier-960px.webp" width=100% title="Parts of a slide are magnified to show details of a figure.">
</td>
<td>
PDF documents can include videos, sounds, slide transitions and some primitive animations. These features are partially supported by BeamerPresenter.
<img srcset=".readme/video-640px.webp 640w, .readme/video-960px.webp 960w" sizes="(max-width: 640px) 640px, 960px" src=".readme/video-960px.webp" width=100% title="Slide showing a video. The speaker window additionally shows a slider to control the video.">
</td>
</tr>
</table>



## Installation
In Arch Linux you can install one of the AUR packages [beamerpresenter](https://aur.archlinux.org/packages/beamerpresenter) and [beamerpresenter-git](https://aur.archlinux.org/packages/beamerpresenter-git).
Note that in these packages by default MuPDF is selected as PDF engine.
In the PKGBUILD file of beamerpresenter-git you can easily adjust the Qt version and the PDF engine.

You can try to install the automatically built flatpak package .
The flatpak package is currently only available for Qt 5.15 and Poppler as PDF engine.
The flatpak package is new, there might be some bugs.

To install the flatpak package you first need the runtime `org.kde.Platform/x86_64/5.15-21.08` (if you need a different architecture or Qt version you can build it your self or open an issue).
```sh
flatpak install org.kde.Platform/x86_64/5.15-21.08
```
Then you can download and unzip the output of the workflow: [![Flatpak](https://github.com/stiglers-eponym/BeamerPresenter/actions/workflows/flatpak-builder.yml/badge.svg)](https://github.com/stiglers-eponym/BeamerPresenter/actions/workflows/flatpak-builder.yml).
When clicking on the latest run you should find a link named `beamerpresenter-x86_64`. This lets you download `beamerpresenter-x86_64.zip`, which you should unzip to obtain `beamerpresenter.flatpak`.
Unfortunately downloading action artifacts seems restricted to users who are logged in to GitHub.
The file `beamerpresenter.flatpak` can be installed using flatpak:
```sh
flatpak install beamerpresenter.flatpak
```

## Manual installation
Building is tested in Arch Linux and in xubuntu 20.04.
Older versions of ubuntu are only compatible with version 0.1 of BeamerPresenter which is described
[here](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x#build).

The build process has changed recently. Building the releases up to 0.2.1 is very different from building the current git version. The releases use qmake, the mainline version uses cmake.

First install the dependencies.
You need cmake, zlib and Qt 5/6 including the multimedia module.
In Qt 5 versions since 5.12 are tested, but other versions starting from 5.9 should also be supported. For installation in Qt 6 you need version 6.2 (or later).
Additionally you need either the Qt 5/6 bindings of poppler or the MuPDF C bindings.

### Dependencies in Ubuntu 20.04
* `cmake` (only for building and only in the mainline version)
* `zlib1g-dev` (after the installation you can remove `zlib1g-dev` and keep only `zlib1g`)
* `qtmultimedia5-dev` (after the installation you can remove `qtmultimedia5-dev` and keep only `libqt5multimedia5` and `libqt5multimediawidgets5`)
* `libqt5multimedia5-plugins` (optional, for showing videos)

For poppler (optional, see [below](#build-with-qmake-releases)):
* `libpoppler-qt5-dev`: version 0.86.1 is tested. Versions below 0.70 are explicitly not supported, problems with newer versions might be fixed if reported in an issue on github. (after the installation you can remove `libpoppler-qt5-dev` and keep only `libpoppler-qt5-1`

For MuPDF (optional, see [below](#build-with-qmake-releases)):
* `libmupdf-dev` (only for building): Tested versions: 1.16.1 - 1.19.0.
* `libfreetype-dev` (after the installation you can remove `libfreetype-dev` and keep only `libfreetype6`)
* `libharfbuzz-dev` (after the installation you can remove `libharfbuzz-dev` and keep only `libharfbuzz0b`)
* `libjpeg-dev` (after the installation you can remove `libjpeg-dev` and keep only `libjpeg8`)
* `libopenjp2-7-dev` (after the installation you can remove `libopenjp2-7-dev` and keep only `libopenjp2-7`)
* `libjbig2dec0-dev` (after the installation you can remove `libjbig2dec0-dev` and keep only `libjbig2dec0`)
* `libgumbo-dev` (for MuPDF >=1.18, probably not for version <=1.17; after the installation you can remove `libgumbo-dev` and keep only `libgumbo1`)

### Dependencies in Arch Linux
Replace qt5 by qt6 in all package names if you want to use Qt 6.
* `cmake` (only for building and only in the mainline version)
* `qt5-multimedia` (depends on `qt5-base`, which is also required)
* optional: `qt5-svg` for showing icons

For poppler (optional, see [below](#build-with-qmake-releases)):
* `poppler-qt5`

For MuPDF (optional, see [below](#build-with-qmake-releases)):
* `libmupdf` (only for building, tested versions: 1.16.1 - 1.19.0)
* `jbig2dec`
* `openjpeg2`
* `gumbo-parser`

Optional, for showing videos:
* `gst-libav`
* `gst-plugins-good`

### Build with qmake (releases)
This is the build process for release 0.2.1 (or older).
For the mainline version follow the instructins [below](#build-with-cmake-mainline).

Download the sources:
```sh
wget https://github.com/stiglers-eponym/BeamerPresenter/archive/v0.2.1.tar.gz
sha256sum -c - <<< "3876bea71907aa64766cff6f7da6fd3bb50a89325e8dba64618a594e1749ed42 v0.2.1.tar.gz"
tar -xf v0.2.1.tar.gz
cd BeamerPresenter-0.2.1
```
On systems other than GNU+Linux you now need to configure libraries in `beamerpresenter.pro`.
But note that if you want to put some effort in configuring a build for a different OS, you should consider directly using the current mainline version that uses cmake instead of qmake.

When building you need to **define the PDF engine**.
Build with one of the following commands: (for Qt 6 you might need to replace `qmake` by `qmake6`)
```sh
qmake -config release RENDERER=mupdf && make
qmake -config release RENDERER=poppler && make
```
To build with debug information you can add `CONFIG+=debug` as an arguments to qmake.
If this fails and you have all dependencies installed, you should check your
Qt version (`qmake --version`) and
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github.
In older versions you may also open an issue, but it will probably not be fixed.

In GNU+Linux you can now install BeamerPresenter with
```sh
make install
```
For packaging the environment variable `$INSTALL_ROOT` may be helpful.

### Build with cmake (mainline)
Clone the git repository
```sh
git clone --depth 1 --single-branch https://github.com/stiglers-eponym/BeamerPresenter.git
```
On systems other than GNU+Linux you now need to configure libraries etc. in `CMakeLists.txt`.
Pull requests or issues with build instructions for other systems are welcome!

The command line for configuring cmake can look like this:
```sh
cmake \
    -B "build-dir" \ # build directory
    -S "BeamerPresenter" \ # source directory
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_POPPLER=ON \
    -DUSE_MUPDF=OFF \
    -DQT_VERSION_MAJOR=6 \
    -DCREATE_SHARED_LIBRARIES=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_SYSCONFDIR=/etc
```
The most important options here are:
* `-DCMAKE_BUILD_TYPE`: Enable debugging by setting this to `Debug` instead of `Release`.
* `-DUSE_POPPLER`: enable PDF engine Poppler. The Poppler library and its Qt 5/6 wrapper must be available. Valid values are `ON` and `OFF`.
* `-DUSE_MUPDF`: enable PDF engine MuPDF. The MuPDF static library and headers as well as the other dependencies listed above must be available. Valid values are `ON` and `OFF`.
* `-DQT_VERSION_MAJOR`: Qt version, must be set manually! Valid values are `5` and `6`.
* `-DCMAKE_INSTALL_PREFIX`: path for package installation.
* `-DCMAKE_INSTALL_SYSCONFDIR`: path for installation of package configuration.

Some arguments for choosing MuPDF or Poppler:
* MuPDF produces a much larger package size. BeamerPresenter with MuPDF can be twice as large as the full poppler installation.
* MuPDF may have better performance.
* My impression is that in most cases MuPDF produces slightly better-looking slides than Poppler. But that may depend on the program used to create the PDF, the fonts, the resolution ...
* Enabling both is possible but not recommended, because it leads to program crashes when using Poppler for some documents.
* Some special case of audio files linked from a PDF is currently only handled correctly when using Poppler.

After configuring with cmake you can build the project (hint: add ` -j 4` for compiling with 4 CPU cores)
```sh
cmake --build build-dir
```
Then install the package. For packaging the environment variable `$DESTDIR` may be helpful.
```sh
cmake --install build-dir
```


## Bugs
If you find bugs or have suggestions for improvements, please
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues).

When reporting bugs, please include the version string of BeamerPresenter
(`beamerpresenter --version`) or the Qt version if you have problems building
BeamerPresenter (`qmake --version`).


## Development

### Known problems
#### Multimedia
* Video performance is bad in the following situations:
    * when you draw or erase on a slide while a video is played (drawing one long stroke is much worse than many short strokes)
    * when you use the magnifier or torch while a video is played when there are also drawings on the same slide. The magnifier generally has rather limited performance when used on a slide that contains drawings.
    * when using multiple magnifiers (yes, that's possible, but you shouldn't do it)
* Sound links in PDFs are unsupported in the MuPDF version. They should work with Poppler.
* Embedded sounds are unsupported, only links to external files can be played.
#### User interface
* Tool buttons can be changed in the user interface, but these changes are not saved. Buttons are part of the user interface, which can only be changed (permanently) by editing the JSON-formatted configuration file.
#### Drawing/annotating
* fixed in current git version: undo after clearing a text field may lead to a crash of the program.
* The undo/redo actions do not handle text annotations correctly. They only remove or create text fields. But when editing a text field the default keyboard shortcuts (CTRL+Z and CTRL+SHIFT+Z) can be used to undo/redo changes in the text. Deleting a text field by deleting its entire text cannot be undone.
* The detection of unsaved changes is quite unreliable. When closing BeamerPresenter you may sometimes see a warning of possibly unsaved changes although there are no unsaved changes. This warning is currently not shown when closing the window through the window manager. The warning can be avoided by using the action "quit unsafe" instead of "quit".
#### Slide transitions
* Sometimes the slides are not automatically rendered to the correct size when resizing the window. Changing or updating the page should solve this problem.
* Some slide transitions need to stop videos. Fly slide transitions during videos can cause strange effects.
* If a preview shows specific overlays, slide changes adding or removing synchronization of this preview with an another widget may lead to short flickering. Slide transitions during such slide changes can contain some ugly artifacts.
#### Backend
* When compiling with both MuPDF and poppler (`qmake RENDERER=both`), trying to open a PDF with renderer=poppler can result in a segmentation fault for some PDFs (when loading the document or when reaching a certain page). The reason might be a linkage problem with some color-space functions. It is recommended to compile with only one PDF engine.
* The program might crash randomly when resizing windows while quickly changing slides including slides that contain videos or fly transitions. This is currently considered a rarely occurring and not easily reproducible issue.
* Some effects (animations, slide transitions) might not work as expected if (at least) one window showing the slide is currently hidden and not updated by the window manager.
* Fixed in MuPDF 1.19: Slide labels are broken for encrypted PDFs when using MuPDF.


### Ideas for further development
* improve code documentation (can be generated with `doxygen doxyfile`)
* tools to select and modify drawings
* improve cache management and layout corrections.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* make layout more reliable
* improve keyboard shortcuts in other widgets than slide widget
* more icons for actions
* option to insert extra slides for drawing
* improve compatibility with Xournal++
* interface for communication with an external control device


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
