# Changelog
## 0.2.4 (upcoming)
* compatibility with Qt 6.4 and Qt 6.5
* pages listed in TOC always start a new slide and are not considered overlays
* page up and page down can be used to navigate pages also while showing TOC or thumbnails
* more navigation keyboard shortcuts in default configuration
* new widget: ToolWidget ("tools") showing tools currently connected to input devices
* improved search widget: now highlights search results
* different installation path for translation files
* copy and paste to/from image formats:
    * this adds the Qt SVG module as a dependency
    * paste only as picture (no erasing possible)
* bug fixes
* internal: different drawing history, memory management, and stacking order mechanism

## 0.2.3
* select and manipulate drawn items
    * selection tools
    * clipboard (shared clipboard with other applications is experimental)
    * move, resize and rotate drawn objects
    * change tool for existing objects
* new widgets: analog clock and search
* new PDF engine: Qt PDF
* keyboard shortcuts work in all widgets by default
* compatibility with newer versions of MuPDF
* tablet input: handle more buttons of the stylus
* bug fixes

## 0.2.2
* shape selection for draw tools (including a shape recognizer)
* better layout and more icons for buttons
* new buttons to select properties like color, style, and shape of the currently used tool
* better handling of links in PDFs
* added German translation
* new build system: cmake instead of qmake
* various bug fixes

## 0.2.1
* use Qt 5 or Qt 6
* various bug fixes

## 0.2.0
* initial version of completely rewritten program
