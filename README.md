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
* clock and page number widget

#### To be implemented
* immediately update slides to fit window size (currently this is only done when changing the slide or manually updating)
* fix layout, avoid recalculating full layout when clock label changes text
* slide transitions
* multimedia
* select drawing tools
* switch between per-slide and per-overlay drawings
* drawing using other input devices than tablet stylus
* highlighting tools: highlighter, pointer, torch, magnifier
* other widgets:
    * timer (with color indicating progress relative to estimate)
    * page label (and max. label)
    * overview (thumbnail slides, including or excluding overlays)
    * table of contents
    * plain text / markdown notes (editable?)
    * settings (GUI for config files, experimental version already available)
* animations and automatic slide change
* save and load drawings to xopp-like xml format
* cache slides even when size of slides varies
* option to add extra space on slide for drawing
* option to insert extra slides or extra space below slide for drawing
* option to decouple notes on presentation and speaker screen (-> notes only for speaker)
* cache only required slides in previews showing specific overlays
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
