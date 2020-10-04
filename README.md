# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes the
current time, a timer for the presentation, and previews of the next slides.

This branch is in an early stage of completely rewriting BeamerPresenter in a
modular, clean and flexible way.

Already implemented:
* render with Poppler or MuPDF
* cache pages (with limitation of available memory)
* slides and notes in same PDF, side by side on same page
* navigation links inside document
* navigation skipping overlays
* build flexible GUI from config (needs improvement!)
* draw and erase using tablet input device with variable pressure (wacom)
* full per-slide history of drawings (with limitation of number of history steps)

To be implemented:
* slide transitions
* multimedia
* select drawing tools
* switch between per-slide and per-overlay drawings
* drawing using other input devices than tablet stylus
* highlighting tools: highlighter, pointer, torch, magnifier
* better, automatically updating GUI (mainly slide geometry)
* other widgets:
    * clock
    * timer (with color indicating progress relative to estimate)
    * page number (and page label, total number of pages)
    * overview (thumbnail slides, including or excluding overlays)
    * table of contents
    * plain text / markdown notes (editable?)
    * settings (own window, GUI for config files)
    * help
* switch between different modes in GUI
* animations and automatic slide change
* save and load drawings
* option to add extra space on slide for drawing
* option to insert extra slide for drawing
* option to decouple notes on presentation and speaker screen (-> notes only for speaker)
