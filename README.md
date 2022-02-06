# BeamerPresenter
BeamerPresenter is a PDF viewer for presentations, which opens a presentation
screen and a control screen in two different windows. The control screen
optionally shows slides from a dedicated notes document instead of the slides
for the audience. Additional information on the control screen includes a
clock, a timer for the presentation, and previews of the next slides.

This software uses the Qt framework and the PDF engines MuPDF or poppler.

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
There exist different flavors of BeamerPresenter:
You can choose the PDF engine (Poppler or MuPDF) and the major Qt version (5 or 6).
Some arguments for this choice and the manual installation are explained [here](https://github.com/stiglers-eponym/BeamerPresenter/blob/main/INSTALL.md).

In Arch Linux and Manjaro you can install one of the AUR packages [beamerpresenter](https://aur.archlinux.org/packages/beamerpresenter) and [beamerpresenter-git](https://aur.archlinux.org/packages/beamerpresenter-git).
Note that in these packages by default MuPDF is selected as PDF engine.

There exists also a package for Nix (thanks to the maintainer of that one!) which can be installed with
```sh
nix-env -iA nixos.beamerpresenter    # on NixOS
nix-env -iA nixpkgs.beamerpresenter  # on non-NixOS
```

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
The build process for these packages is explained [here](https://github.com/stiglers-eponym/BeamerPresenter/tree/main/packaging).


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
(`beamerpresenter --version`).


## Development

### Known problems
#### Multimedia
* Video performance is bad in the following situations:
    * when you draw or erase on a slide while a video is played (drawing one long stroke is much worse than many short strokes)
    * when you use the magnifier or torch while a video is played when there are also drawings on the same slide. The magnifier generally has rather limited performance when used on a slide that contains drawings.
    * when using multiple magnifiers (yes, that's possible, but you shouldn't do that)
* Embedded sounds are unsupported, only links to external files can be played.
* Sounds included as sound link (not sound annotation) are unsupported when using MuPDF.
#### User interface
* Tool buttons can be changed in the user interface, but these changes are not saved. Buttons are part of the user interface, which can only be changed (permanently) by editing the JSON-formatted configuration file.
#### Drawing/annotating
* The detection of unsaved changes is not reliable. The warning when closing beamerpresenter with unsaved changes can be avoided by using the action "quit unsafe" instead of "quit".
#### Slide transitions
* Sometimes the slides are not automatically rendered to the correct size when resizing the window. Changing or updating the page should solve this problem.
* Some slide transitions need to stop videos. Fly slide transitions during videos look strange.
* If a preview shows specific overlays, slide changes adding or removing synchronization of this preview with another widget may lead to short flickering. Slide transitions during such slide changes can contain some ugly artifacts.
#### PDF backend
* When compiling with both MuPDF and poppler, trying to open a PDF with renderer=poppler can result in a segmentation fault for some PDFs (when loading the document or when reaching a certain page). The reason might be a linkage problem with some colorspace functions. It is recommended to compile with only one PDF engine.
* In MuPDF only a smaller subset of PDF link types is supported (only internal navigation links and external links) as compared to the Poppler version.
* Some effects (animations, slide transitions) might not work as expected if (at least) one window showing the slide is currently hidden and not updated by the window manager.
* Fixed in MuPDF 1.19: Slide labels are broken for encrypted PDFs when using MuPDF.


### Ideas for further development
* tools to select and modify drawings
* improve cache management and layout corrections.
* cache slides even when size of slides varies (partially implemented)
* cache only required slides in previews showing specific overlays
* make layout more reliable
* improve keyboard shortcuts in other widgets than slide widget
* option to insert extra slides for drawing


## License
This software may be redistributed and/or modified under the terms of the GNU Affero General Public License (AGPL), version 3, available on the [GNU web site](https://www.gnu.org/licenses/agpl-3.0.html). This license is chosen in order to ensure compatibility with the software libraries used by BeamerPresenter, including Qt, MuPDF, and poppler.

BeamerPresenter can be compiled without including MuPDF, using only poppler as a PDF engine.
Those parts of the software which can be used without linking to MuPDF may, alternatively to the AGPL, be redistributed and/or modified under the terms of the GNU General Public License (GPL), version 3 or any later version, available on the [GNU web site](https://www.gnu.org/licenses/gpl-3.0.html).

BeamerPresenter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.
