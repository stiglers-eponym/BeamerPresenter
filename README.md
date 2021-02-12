# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes the
current time, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF and/or poppler.

**Note**: the relatively stable version 0.1.x can be found in [this branch](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x) and in the [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases). This new version 0.2.x is not stable yet.

## Features
* modular user interface (new in 0.2.x)
* render slides to compressed cache for fast response
* poppler or MuPDF (new in 0.2.x) as PDF engine
* draw in slides (improved in 0.2.x)
* highlighting tools (torch, magnifier, pointer)
* notes for the speaker in Markdown format (new in 0.2.x)
* optionally to show separate file for speaker
* optionally use LaTeX-beamer's option to show notes on second screen and split PDF pages into a part for the speaker and a part or the audience
* clock and timer for the presentation (some features of 0.1.x not yet available in 0.2.x)
* simple navigation using document outline, thumbnail slides, or navigation by page label

### Currently NOT supported in version 0.2.x
* multimedia content on slides (only 0.1.x)
* slide transitions (only 0.1.x)
* animations (only 0.1.x)
* save/load drawings (only 0.1.x)


## Build and install
**Notes**: building and installing the [releases](https://github.com/stiglers-eponym/BeamerPresenter/releases) of version 0.1.x is described [here](https://github.com/stiglers-eponym/BeamerPresenter/tree/0.1.x#build).

Building is tested in an up-to-date Arch Linux and (from time to time) in ubuntu 20.04.
Older versions of ubuntu are not supported, because ubuntu 18.04 uses old versions of poppler and MuPDF and other ubuntu versions before 20.04 should not be used anymore.
Version 0.1.x of BeamerPresenter should support ubuntu 18.04 and you should
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues)
on github if it does not.

First install required packages. You need Qt5 including the multimedia module.
Additionally you need either the Qt5 bindings of poppler or the MuPDF C bindings.

### Dependencies in Ubuntu 20.04
For Qt5:

* `qt5-qmake`
* `qt5-default`
* `qtmultimedia5-dev`
* Note: ubuntu's version of Qt5 does not have native markdown support. Therefore, also BeamerPresenter will not be able to interpret markdown in ubuntu.

For poppler:

* `libpoppler-qt5-dev`: version 21.01 is tested. Versions below 0.70 are explicitly not supported, compiler errors in newer versions might be fixed if reported in an issue on github.

For MuPDF:

* `libmupdf-dev`: MuPDF versions starting from 1.17 should work, version 1.12 or older is explicitly not supported.
* `libfreetype-dev`
* `libharfbuzz-dev`
* `libjpeg-dev`
* `libopenjp2-7-dev`
* `libjbig2dec0-dev`
* `libgumbo-dev` (for MuPDF 1.18, probably not in 1.17)

### Dependencies in Arch Linux
For Qt5:
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
Now you need to select the PDF engine. In the file `beamerpresenter.pro`
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
* `config/beamerpesenter.conf`: settings file, usually stored in `$HOME/.config/beamerpresenter/beamerpresenter.config`. If located at a different path, specify the path with the command line option `-c <path>`.
* `config/gui.json`: user interface configuration, mandatory! Usually stored in `$HOME/.config/beamerpresenter/gui.json`. The path to this file needs to be specified in the settings file (see above) or given explicitly with the command line option `-g <path>`.
* `doc/*`: manuals, not required for running the program.
* `share/*`: icon and desktop file, not required for running the program.
* `LICENSE` (optional): you might want to remember that it's AGPL3 (see [below](https://github.com/stiglers-eponym/BeamerPresenter#license) for details)


## Development
Version 0.2.x of BeamerPresenter has been developed independent of version 0.1.y
with the aim of avoiding the chaotic old code.
The configuration files of version 0.2.x and 0.1.x are incompatible.

#### Already implemented
* render with Poppler or MuPDF
* cache pages (with limitation of available memory; doesn't work if pages have different sizes)
* read slides and notes from the same PDF, side by side on same page
* navigation links inside document
* navigation skipping overlays
* build flexible GUI from config
* draw and erase using tablet input device with variable pressure (wacom)
* highlighting tools: pointer, torch, magnifier
* full per-slide history of drawings (with limitation of number of history steps)
* widgets:
    * slide
    * clock
    * page number (and max. number)
    * page label (and max. label)
    * tool selector
    * timer
    * editable markdown notes per page label (only available if qt was compiled with native markdown implementation)
    * table of contents (requires improvement: keyboard navigation)
    * thumbnails (requires improvement: keyboard navigation)
    * settings (still kind of improvised)

#### To be implemented
* improve cache management and layout corrections: sometimes cache is not used correctly.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* fix layout, avoid recalculating full layout when clock label changes text
* immediately update slides to fit window size (currently this is only done when changing the slide or manually updating)
* animations and automatic slide change
* slide transitions
* multimedia
* option to insert extra slides or extra space below slide for drawing (partially implemented)
* switch between per-slide and per-overlay drawings
* improve text input tool
* save and load drawings to xopp-like xml format
* improve widgets:
    * settings (more explanations)
    * timer (with color indicating progress relative to estimate like in version 0.1.x)
    * thumbnails (cursor, keyboard navigation)
    * table of contents (cursor, keyboard navigation)
    * all: keyboard shortcuts
* fine-tuned interface, fonts, ...
* manual, man pages


## Bugs
If you find bugs or have suggestions for improvements, please
[open an issue](https://github.com/stiglers-eponym/BeamerPresenter/issues).

When reporting bugs, please include the version string of BeamerPresenter
(`beamerpresenter --version`) or the Qt version if you have problems building
BeamerPresenter (`qmake --version`).


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
