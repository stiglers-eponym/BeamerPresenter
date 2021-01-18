# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes the
current time, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF and/or poppler.


## Development
This branch is in an early stage of completely rewriting BeamerPresenter in a
modular, clean and flexible way.

#### Already implemented
* render with Poppler or MuPDF
* cache pages (with limitation of available memory; doesn't work if pages have different sizes)
* read slides and notes from the same PDF, side by side on same page
* navigation links inside document
* navigation skipping overlays
* build flexible GUI from config (not really stable)
* draw and erase using tablet input device with variable pressure (wacom)
* full per-slide history of drawings (with limitation of number of history steps)
* widgets:
    * slide
    * clock
    * page number
    * page label (and max. label)
    * tool selector
    * timer
    * editable markdown notes per page label
    * table of contents (still kind of improvised)
    * thumbnails (still kind of improvised)

#### To be implemented
* improve cache management and layout corrections: sometimes cache is not used correctly.
* avoid fatal error when stopping program while cache threads are running
* cache slides even when size of slides varies
* cache only required slides in previews showing specific overlays
* fix layout, avoid recalculating full layout when clock label changes text
* immediately update slides to fit window size (currently this is only done when changing the slide or manually updating)
* animations and automatic slide change
* slide transitions
* multimedia
* option to insert extra slides or extra space below slide for drawing
* switch between per-slide and per-overlay drawings
* highlighting tools: pointer, torch, magnifier
* save and load drawings to xopp-like xml format
* improve widgets:
    * settings (GUI for config files, experimental version already available)
    * timer (with color indicating progress relative to estimate like in version 0.1.x)
    * thumbnails (cursor)
    * table of contents (kind ov improvised)
    * all: keyboard shortcuts
* fine-tuned interface, fonts, ...
* manual, man pages


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, when using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

This is an early development version. It is NOT READY FOR USAGE.
