# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes a
clock, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF and/or poppler.

## Which version?
Versions [0.1](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) and 0.2 are different programs with incompatible configurations.
[Version 0.1.3](https://github.com/stiglers-eponym/BeamerPresenter/releases) is relatively stable and can be [built in macOS](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x#building-in-macos) without manually adapting beamerpresenter.pro.
Version 0.2.0 has many new features and is much more flexible.
If you want to draw in the presentation using a tablet or drawing pad, or if you want to adapt the interface or use more than two monitors, you should try the current git version (0.2.0).

## Features (selection)
* modular user interface (new in 0.2.x): adapt for your presentation style and technical equipment (number of monitors, extra information available, input devices, ...)
* compressed cache for fast slide changes
* draw in slides, save drawings in a format compatible with Xournal++ (improved in 0.2.x)
* highlighting tools (torch, magnifier, pointer)
* notes for the speaker in Markdown format (new in 0.2.x)
* (optionally) show separate presentation file for speaker or use LaTeX-beamer's option to show notes on second screen (split PDF pages into a part for the speaker and a part or the audience)
* time indicates progress relative to a predefined schedule by it's color (improved in 0.2.x)
* navigate using document outline, thumbnail slides, page numbers/labels and links
* videos in presentations (currently without audio)
* slide transitions


## Screenshots
These screenshots only show one possible way of using BeamerPresenter. The speaker could also see a different presentation (with additional information), or editable notes.

One possible configuration of the graphical interface shows the previews of the last overlays of the current and next slide to the speaker:
<img src=".readme/fly-transition.png" width=100% title="Picture flying in during slide transition">
The left half of these pictures shows the window visible only to the speaker and the right half shows the presentation. Both are independent windows.
These examples were created with the configuration `config/gui-2files.json`, which also works fine if just one presentation file is given.

Use a magnifier to show details of your figures (size and magnification factor can be adjusted):
<img src=".readme/magnifier.png" width=100% title="Magnifier (size and magnification factor are adjustable)">

Draw in your presentation using a pen or highlighter and focus on parts of your slide using a torch or a pointer:
<img src=".readme/drawings+torch+pointer.png" width=100% title="Annotations, torch and pointer for highlighting">
Of course, this is just for demonstration an you will usually not use pointer and torch at the same time.
Annotations, notes and some file-specific settings can be saved and loaded to gzipped xml files in a format which aims at compatibility with Xournal++.

Use an overview of all slides (or all slides with separate slide labels, especially useful for presentations created with LaTeX beamer):
<img src=".readme/overview+video.png" width=100% title="Overview mode and video in presentation">

You can embed videos in your slides. Drawing and highlighting also works in the video.
<img src=".readme/draw-video.png" width=100% title="Drawings (pen and highlighter) and torch in video">
Note the slider for the video on the speaker's screen.
Slide transitions may in some cases need to interrupt a video.

The interface is very flexible and can be adjusted to your needs. Also multiple monitors are possible. Example of 4 windows for 4 different monitors (3 for the audience, 1 for the speaker):
<p align="center">
  <img src=".readme/multi-monitor.png" width=75% title="Example multi-monitor setup: 3 windows for audience, 1 window for speaker">
</p>


## Build and install
**Note**: building and installing version 0.1.3 is described
[here](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x#build).

Building is tested in an up-to-date Arch Linux and (from time to time) in xubuntu 20.04.
Older versions of ubuntu are not supported, because ubuntu 18.04 uses old versions of poppler and MuPDF and other ubuntu versions before 20.04 should not be used anymore.
Version 0.1.x of BeamerPresenter should run in ubuntu 18.04 and you should
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github if it does not.

In Arch Linux you can install `beamerpresenter-git` from the AUR.
By default MuPDF is selected as PDF engine in  `beamerpresenter-git`, the configuration for poppler is commented out in the PKGBUILD file.

First install the dependencies. You need Qt 5 including the multimedia module (which is not available in Qt 6.0).
Additionally you need either the Qt 5 bindings of poppler or the MuPDF C bindings.

### Dependencies in Ubuntu 20.04
For Qt 5:
* `qt5-qmake`
* `qt5-default`
* `qtmultimedia5-dev`
* `libqt5multimedia5-plugins` (optional, for showing videos)
* Note: ubuntu's Qt 5 package does not have native markdown support. Therefore, also BeamerPresenter will not be able to interpret markdown in ubuntu.

For poppler (optional, see [below](https://github.com/stiglers-eponym/BeamerPresenter#build)):
* `libpoppler-qt5-dev`: version 0.86.1 is tested. Versions below 0.70 are explicitly not supported, problems with newer versions might be fixed if reported in an issue on github.

For MuPDF (optional, see [below](https://github.com/stiglers-eponym/BeamerPresenter#build)):
* `libmupdf-dev` (only for building): MuPDF versions starting from 1.17 should work, version 1.12 or older is explicitly not supported.
* `libfreetype-dev`
* `libharfbuzz-dev`
* `libjpeg-dev`
* `libopenjp2-7-dev`
* `libjbig2dec0-dev`
* `libgumbo-dev` (for MuPDF 1.18, probably not for version 1.17)

Others:
* `zlib1g-dev`

### Dependencies in Arch Linux
For Qt 5:
* `qt5-multimedia` (depends on `qt5-base`, which is also required)

For poppler (optional, see [below](https://github.com/stiglers-eponym/BeamerPresenter#build)):
* `poppler-qt5`

For MuPDF (optional, see [below](https://github.com/stiglers-eponym/BeamerPresenter#build)):
* `libmupdf` (only for building)
* `jbig2dec`
* `openjpeg2`
* `gumbo-parser`

Optional, for showing videos:
* `gst-libav`
* `gst-plugins-good`

### Build
Download the sources:
```sh
git clone --depth 1 https://github.com/stiglers-eponym/BeamerPresenter.git
```
On systems other than GNU+Linux you now need to configure libraries in
`beamerpresenter.pro`. Pull requests or issues with build instructions for
other systems are welcome!

When building you need to **define the PDF engine**.
Build with one of the following commands:
```sh
qmake RENDERER=mupdf && make
qmake RENDERER=poppler && make
```
If this fails and you have all dependencies installed, you should check your
Qt version (`qmake --version`). If you use 5.8 < qt < 6, you should
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github.
In older versions you may also open an issue, but it will probably not be fixed.

### Install
In GNU+Linux you can install BeamerPresenter with
```sh
make install
```

### Upgrade from version 0.1.x
The configuration files of versions 0.1.x and 0.2.x are incompatible.
When upgrading, you should move your configuration files of version 0.1.x to some backup to avoid conflicts.


## Bugs
If you find bugs or have suggestions for improvements, please
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues).

When reporting bugs, please include the version string of BeamerPresenter
(`beamerpresenter --version`) or the Qt version if you have problems building
BeamerPresenter (`qmake --version`).

## Known problems
* Video lags when drawing on it.
    * Sometimes this can be reduced by first making sure that the presentation window has focus and then pausing and playing the video.
    * In general I think the quality of the videos is quite good, even when using the magnifier.
* Tool buttons can be changed in the user interface, but these changes are not saved. Buttons are part of the user interface, which can only be changed (permanently) by editing the JSON-formatted configuration file.
* Slide labels are broken for encrypted PDFs when using MuPDF.
* Sound in videos is basically untested and currently has low priority. Feel free to open an issue if this is relevant for you.
* When compiling with both MuPDF and poppler (`qmake RENDERER=both`), trying to open a PDF with renderer=poppler can result in a segmentation fault for some PDFs. The reason might be a linkage problem with some color-space functions. It is recommended to compile with only one PDF engine.
* The detection of unsaved changes is quite unreliable. When closing BeamerPresenter you may sometimes see a warning of possibly unsaved changes although there are no unsaved changes. This warning is currently not shown when closing the window through the window manager. The warning can be avoided by using the action "quit unsafe" instead of "quit".
* Sometimes the slides are not automatically rendered to the correct size when resizing the window. Changing or updating the page should solve this problem.
* Some slide transitions need to stop videos. Fly slide transitions during videos can cause strange effects.
* Some slide transitions can show artifacts on preview slide widgets which only show the first or last overlay of a slide.
* Some slide transitions may have bad performance (low frame rate).


## Development

#### Already implemented
* render with Poppler or MuPDF
* cache pages (with limitation of available memory; doesn't work if pages have different sizes)
* read slides and notes from the same PDF, side by side on same page
* navigation links inside document
* navigation skipping overlays
* build flexible GUI from config
* draw and erase using tablet input device with variable pressure
* highlighting tools: pointer, torch, magnifier
* full per-slide history of drawings (with limitation of number of history steps)
* select per-slide or per-overlay drawings
* save and load drawings to xopp-like gzipped xml format
* animations and automatic slide change
* slide transitions
* videos
* add extra space below a slide for drawing
* widgets:
    * slide
    * clock
    * page number (and max. number)
    * page label (and max. label)
    * tool selector
    * timer
    * editable markdown notes per page label or per page number (only available if qt was compiled with native markdown implementation)
    * table of contents (requires improvement: keyboard navigation)
    * thumbnails
    * settings

#### To be implemented / fixed
* strange probabilistic crash at startup
* improve cache management and layout corrections: sometimes cache is not used correctly.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* make layout more reliable
* sounds, mute/unmute actions
* tools to select and modify drawings
* option to insert extra (blank or copied) slides for drawing
* improve text input tool, fix bugs arising when using text input tool with "special" devices
* combination of slide transitions and videos sometimes interrupts videos
* improve widgets:
    * thumbnails (cursor, keyboard navigation)
    * table of contents (cursor, keyboard navigation)
    * all: keyboard shortcuts
* fine-tuned interface, fonts, ...


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
