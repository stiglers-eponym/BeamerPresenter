# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which shows presentation
slides in one window and notes for the speaker (or the same presentation
slides) with some additional information in a second window.

BeamerPresenter is mainly tested with presentations created by LaTeX beamer and
tries to implement all features available in LaTeX beamer.

BeamerPresenter is only tested on GNU/Linux with X11 and wayland.


## Build
To build and use this project you need to have the Qt5 multimedia module and the
poppler Qt5 bindings installed.

Download, compile and install this project on GNU/Linux systems:
```sh
git clone https://github.com/stiglers-eponym/BeamerPresenter.git
cd BeamerPresenter
qmake && make
make install
```
Showing videos in a presentation additionally requires the installation of some
GStreamer plugins.

### Installation on Arch Linux
You can install the package beamerpresenter from the AUR.

### Installation on Ubuntu >= 18.04
Note: BeamerPresenter requires Qt >= 5.9 (versions 5.6 - 5.8 are untested), which is only provided by ubuntu >= 18.04.

First install the dependences (note that this changes the default Qt version to Qt 5):
```sh
sudo apt install qt5-qmake qt5-default libpoppler-qt5-dev qtmultimedia5-dev
```
Optionally install `libqt5multimedia5-plugins` for multimedia content.

Then download the source and build:
```sh
git clone https://github.com/stiglers-eponym/BeamerPresenter.git
cd BeamerPresenter
qmake && make
```
You can install BeamerPresenter using `sudo make install`, but it is recommended to use `checkinstall`
```sh
sudo apt install checkinstall
echo 'Simple dual screen pdf viewer' > description-pak
sudo checkinstall -D --pkglicense=GPL3 --requires=libpoppler-qt5,qtmultimedia5 --pkgsource=github.com/stiglers-eponym/BeamerPresenter make install
```
in order to keep track of all installed files using apt.


## Usage
```sh
beamerpresenter [options] <presentation.pdf> [<notes.pdf>]
```
For more options and usage possibilities use `beamerpresenter --help` and the
man page.

If you start `beamerpresenter` without any arguments, it will show a file dialog
in which you can pick you presentation and notes file.


## Features
An extended list of features can be found in the manual (`beamerpresenter.1`).
*	Speaker screen showing notes, previews of the current and next slide, a
	timer, a clock and the current slide number.
*	The timer can be configured to change its color as to indicate whether you
	are in time.
*	Cache: All slides are rendered to a compressed cache.
	The total cache size and the number of slides in cache can be limited.
*	Slide transitions: BeamerPresenter probably all slide transitions which are
	available for PDFs.
*	Multimedia: You can add videos and audio files to your presentation.
	A video will continue playing if embedded on two consecutive slides with
	the same page label (overlays in LaTeX beamer).
*	Animations: You can create simple animations by showing slides in rapid
	succession. The minimal delay between two frame can be defined in the
	configuration.
*	Simple navigation through links, scrolling, a table of contents and an
	overview of all slides on the speaker's screen and shortcuts for skipping
	overlays.
*	(Experimental:) Embed Applications: If you are using X11, you can show
	windows created by external applications in a presentation.


## Settings
Settings can be placed in a file `beamerpresenter.conf` (on platforms other than
GNU/Linux: `beamerpresenter.ini`). An example configuration file is provided.
The configuration can only be edited directly with a text editor.

This program is only tested on a GNU/Linux system. Using configuration files might
lead to platform specific problems.


## Typical usage
I typically compile my LaTeX beamer presentations once with an aspect ratio
optimized for the projector and once with aspect ratio 4:3 for the speaker's
screen. The layout is less important for the speaker's screen, but it should
leave enough space on the screen for a clock, a timer and a preview of the next
slide.

In the presentation compiled for the speaker's screen I can include additional
notes and hyperlinks to backup slides.
