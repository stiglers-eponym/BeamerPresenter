# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes a
clock, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF and/or poppler.

**Note**: the relatively stable version 0.1.3 can be found in [branch 0.1.x](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) and in the [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases). This new version 0.2.0beta is not stable yet.

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
<img src=".readme/multi-monitor.png" width=60% title="Example multi-monitor setup: 3 windows for audience, 1 window for speaker">


## Build and install
**Note**: building and installing the [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) of version 0.1.x is described [here](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x#build).

Building is tested in an up-to-date Arch Linux and (from time to time) in xubuntu 20.04.
Older versions of ubuntu are not supported, because ubuntu 18.04 uses old versions of poppler and MuPDF and other ubuntu versions before 20.04 should not be used anymore.
Version 0.1.x of BeamerPresenter should support ubuntu 18.04 and you should
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github if it does not.

First install required packages. You need Qt 5 including the multimedia module (which is not available in Qt 6.0).
Additionally you need either the Qt 5 bindings of poppler or the MuPDF C bindings.

### Dependencies in Ubuntu 20.04
For Qt 5:

* `qt5-qmake`
* `qt5-default`
* `qtmultimedia5-dev`
* Note: ubuntu's Qt 5 package does not have native markdown support. Therefore, also BeamerPresenter will not be able to interpret markdown in ubuntu.

For poppler:

* `libpoppler-qt5-dev`: version 21.01 is tested. Versions below 0.70 are explicitly not supported, problems with newer versions might be fixed if reported in an issue on github.

For MuPDF:

* `libmupdf-dev`: MuPDF versions starting from 1.17 should work, version 1.12 or older is explicitly not supported.
* `libfreetype-dev`
* `libharfbuzz-dev`
* `libjpeg-dev`
* `libopenjp2-7-dev`
* `libjbig2dec0-dev`
* `libgumbo-dev` (for MuPDF 1.18, probably not in 1.17)

Others:
* `zlib1g-dev`

### Dependencies in Arch Linux
For Qt 5:
* `qt5-multimedia` (depends on `qt5-base`, which is also required)

For poppler:
* `poppler-qt5`

For MuPDF:
* `libmupdf`
* `libfreetype.so` (provided by `freetype2` in test environment)
* `libharfbuzz.so` (provided by `harfbuzz` in test environment)
* `libjpeg` (provided by `libjpeg-turbo` in test environment)
* `jbig2dec`
* `openjpeg2`
* `gumbo-parser`

### Build

Download the sources:
```sh
git clone --depth 1 https://github.com/stiglers-eponym/BeamerPresenter.git
```
Now you need to **select the PDF engine**. In the file `beamerpresenter.pro`
you will find the lines
`DEFINES += INCLUDE_POPPLER` and
`DEFINES += INCLUDE_MUPDF`.
Comment out the PDF engine which you don't need with a `#`.

Now you can start building.
```sh
qmake && make
```
If this fails and you have all dependencies installed, you should check your
Qt version (`qmake --version`). If you use 5.8 < qt < 6, you should
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github.
In older versions you may also open an issue, but it will probably not be fixed.

### Install
In GNU/Linux you can install BeamerPresenter with
```sh
make install
```
When installing manually, you may need the following files (in build directory):

* `beamerpesenter`: executable, the program
* `config/beamerpesenter.conf`: settings file, usually stored in `/etc/xdg/beamerpresenter/beamerpresenter.conf` or `$HOME/.config/beamerpresenter/beamerpresenter.conf`. If located at a different path, specify the path with the command line option `-c <path>`.
* `config/gui.json`: user interface configuration, mandatory! Usually stored in `/etc/xdg/beamerpresenter/gui.json` or `$HOME/.config/beamerpresenter/gui.json`. The path to this file needs to be specified in the settings file (see above) or given explicitly with the command line option `-g <path>`.
* `doc/*`: manuals, not required for running the program.
* `share/*`: icon, desktop file and in-app manual, not required for running the program.
* `LICENSE` (optional): you might want to remember that it's AGPL3 (see [below](https://github.com/stiglers-eponym/BeamerPresenter#license) for details)

### Upgrade from version 0.1.x
The configuration files of versions 0.1.x and 0.2.x are incompatible.
When upgrading, you should move your config files from version 0.1.x to some backup to avoid conflicts.


## Bugs
If you find bugs or have suggestions for improvements, please
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues).

When reporting bugs, please include the version string of BeamerPresenter
(`beamerpresenter --version`) or the Qt version if you have problems building
BeamerPresenter (`qmake --version`).


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
* improve cache management and layout corrections: sometimes cache is not used correctly.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* fix layout, avoid recalculating full layout when clock label changes text
* sounds
* tool to select and modify drawings
* option to insert extra (blank or copied) slides for drawing
* improve text input tool
* combination of slide transitions and videos sometimes interrupts videos
* make xml file loading more stable
* improve widgets:
    * thumbnails (cursor, keyboard navigation)
    * table of contents (cursor, keyboard navigation)
    * all: keyboard shortcuts
* fine-tuned interface, fonts, ...
* manual, man pages


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
