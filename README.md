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
* videos in presentations
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
In Arch Linux and Manjaro you can install one of the AUR packages [beamerpresenter](https://aur.archlinux.org/packages/beamerpresenter) and [beamerpresenter-git](https://aur.archlinux.org/packages/beamerpresenter-git).
Note that in these packages by default MuPDF is selected as PDF engine.

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


## Manual installation
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

### Build
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

Some arguments for choosing MuPDF or Poppler:
* MuPDF produces a much larger package size (37MB instead of 1.3MB in Arch).
* MuPDF may have better performance.
* My impression is that in most cases MuPDF produces slightly better-looking slides than Poppler. But this may depend on presentation, the fonts, the resolution ...
* Enabling both PDF engines is possible but not recommended, because it can lead to program crashes when using Poppler for some documents.
* One case of audio files linked from a PDF is currently only handled correctly when using Poppler.

After configuring with cmake you can build the project (hint: add ` -j 4` for compiling with 4 CPU cores)
```sh
cmake --build build-dir
```
Then install the package. For packaging the environment variable `$DESTDIR` may be helpful.
```sh
cmake --install build-dir
```


## Configuration
There are two different aspects of the configuration:

### Program settings
Settings for the program are configured in the configuration file
`beamerpresenter.conf` as documented in `man 5 beamerpresenter.conf`. Most of
these settings can also be changed in the settings widget in the graphical
interface, but some of these settings require a restart of the program.

Some program settings can be temporarily overwritten using command line
arguments (documented in `beamerpresenter --help' or `man 1 beamerpresenter`).

### User interface
The user interface is configured in a separate JSON file (GUI config, `gui.json`) as
documented in `man 5 beamerpresenter-ui`. This file can not be edited in the
graphical interface.

The GUI config defines which widgets are shown, including the number of windows,
tool buttons, notes for the speaker, combination of different PDF files, and
various other settings. A GUI config file can be selected temporarily with the
command line option `-g <file>`.


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
* handle links external URLs
* tools to select and modify drawings
* improve cache management and layout corrections.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* make layout more reliable
* improve keyboard shortcuts in other widgets than slide widget
* option to insert extra slides for drawing
* improve compatibility with Xournal++


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
