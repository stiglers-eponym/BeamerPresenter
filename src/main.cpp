/*
 * This file is part of BeamerPresenter.
 * Copyright (C) 2019  stiglers-eponym

 * BeamerPresenter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * BeamerPresenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with BeamerPresenter. If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <QSettings>
#include <QApplication>
#include <QFileDialog>
#include <QCommandLineParser>
#include <QKeySequence>
#include "controlscreen.h"
#include "presentationscreen.h"

const QMap<QString, int> keyActionMap {
    {"previous", KeyAction::Previous},
    {"next", KeyAction::Next},
    {"previous current screen", KeyAction::PreviousCurrentScreen},
    {"next current screen", KeyAction::NextCurrentScreen},
    {"previous skipping overlays", KeyAction::PreviousSkippingOverlays},
    {"next skipping overlays", KeyAction::NextSkippingOverlays},
    {"go to page", KeyAction::GoToPage},
    {"go to slide", KeyAction::GoToPage},
    {"last page", KeyAction::LastPage},
    {"last slide", KeyAction::LastPage},
    {"first page", KeyAction::FirstPage},
    {"first slide", KeyAction::FirstPage},
    {"sync from control screen", KeyAction::SyncFromControlScreen},
    {"sync from presentation screen", KeyAction::SyncFromPresentationScreen},
    {"update", KeyAction::Update},

    {"update cache", KeyAction::UpdateCache},
    {"start embedded current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start all embedded", KeyAction::StartAllEmbedded},
    {"start all embedded applications", KeyAction::StartAllEmbedded},
    {"play multimedia", KeyAction::PlayMultimedia},

    {"pause timer", KeyAction::PauseTimer},
    {"reset timer", KeyAction::ResetTimer},
    {"show toc", KeyAction::ShowTOC},
    {"hide overview", KeyAction::HideOverview},
    {"show overview", KeyAction::ShowOverview},
    {"hide toc", KeyAction::HideTOC},
    {"toggle cursor", KeyAction::ToggleCursor},
    {"full screen", KeyAction::FullScreen},
    {"reload", KeyAction::Reload},
    {"quit", KeyAction::Quit}
};

const QMap<int, QList<int>> defaultKeyMap = {
    {Qt::Key_PageUp, {KeyAction::Previous}},
    {Qt::Key_PageDown, {KeyAction::Next}},
    {Qt::Key_Left, {KeyAction::Previous}},
    {Qt::Key_Right, {KeyAction::Next}},
    //{Qt::Key_Left, {KeyAction::PreviousCurrentScreen}},
    //{Qt::Key_Right, {KeyAction::NextCurrentScreen}},
    {Qt::Key_Up, {KeyAction::PreviousSkippingOverlays}},
    {Qt::Key_Down, {KeyAction::NextSkippingOverlays}},
    {Qt::Key_G, {KeyAction::GoToPage}},
    {Qt::Key_End, {KeyAction::LastPage}},
    {Qt::Key_Home, {KeyAction::FirstPage}},
    {Qt::Key_Return, {KeyAction::SyncFromControlScreen}},
    {Qt::Key_Escape, {KeyAction::SyncFromPresentationScreen, KeyAction::HideTOC}},
    {Qt::Key_Space, {KeyAction::Update}},

    {Qt::Key_C, {KeyAction::UpdateCache}},
    {Qt::Key_E, {KeyAction::StartEmbeddedCurrentSlide}},
    {Qt::Key_E+Qt::ShiftModifier, {KeyAction::StartAllEmbedded}},
    {Qt::Key_M, {KeyAction::PlayMultimedia}},

    {Qt::Key_P, {KeyAction::PauseTimer}},
    {Qt::Key_R, {KeyAction::ResetTimer}},
    {Qt::Key_T, {KeyAction::ShowTOC}},
    {Qt::Key_S, {KeyAction::ShowOverview}},
    {Qt::Key_O, {KeyAction::ToggleCursor}},
    {Qt::Key_F, {KeyAction::FullScreen}},
    {Qt::Key_U, {KeyAction::Reload}},
    {Qt::Key_Q, {KeyAction::Quit}}
};


int main(int argc, char *argv[])
{
    qSetMessagePattern("%{time process} %{if-debug}D%{endif}%{if-info}INFO%{endif}%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}%{if-category} %{category}%{endif}%{if-debug} %{file}:%{line}%{endif} - %{message}%{if-fatal} from %{backtrace [depth=3]}%{endif}");

    QApplication app(argc, argv);
    app.setApplicationName("BeamerPresenter");
    //app.setApplicationVersion("0.1");

    // set up command line argument parser
    QCommandLineParser parser;
    parser.setApplicationDescription(
            "\nSimple dual screen pdf presentation software.\n"
            "Default shortcuts:\n"
            "  c                Update cache\n"
            "  e                Start all embedded applications on the current slide\n"
            "  E                Start all embedded applications on all slides\n"
            "  g                Go to page (set focus to page number edit)\n"
            "  m                Play or pause all multimedia content\n"
            "  o                Toggle cursor visbility (only on presentation screen)\n"
            "  p                Pause / continue timer\n"
            "  q                Quit\n"
            "  r                Reset timer\n"
            "  t                Show table of contents on control screen\n"
            "  s                Show overview of all slides on control screen\n"
            "  u                Check if files have changed and reload them if necessary\n"
            "  space            Update layout and start or continue timer\n"
            "  Left, PageUp     Go to previous slide and start or continue timer\n"
            "  Right, PageDown  Go to next slide and start or continue timer\n"
            "  Up               Go to the previous slide until the page label changes.\n"
            "                   In beamer presentations: last overlay of the previous slide.\n"
            "  Down             Go to the next slide until the page label changes.\n"
            "                   In beamer presentations: first overlay of the next slide.\n"
            "  F11, f           Toggle fullscreen (only for current window)\n"
            "  return           Accept the page number from the notes and continue presentation\n"
            "  escape           Go to the note page for the current slide. Hide table of contents.\n"
        );
    parser.addHelpOption();
    //parser.addVersionOption();
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    parser.addPositionalArgument("<notes.pdf>",  "Notes for the presentation (optional, should have the same number of pages as <slides.pdf>)");
    parser.addOptions({
        {{"a", "autoplay"}, "true, false or number: Start video and audio content when entering a slide.\nA number is interpreted as a delay in seconds, after which multimedia content is started.", "value"},
        {{"A", "autostart-emb"}, "true, false or number: Start embedded applications when entering a slide.\nA number is interpreted as a delay in seconds, after which applications are started.", "value"},
        {{"c", "cache"}, "Number of slides that will be cached. A negative number is treated as infinity.", "int"},
        {{"e", "embed"}, "file1,file2,... Mark these files for embedding if an execution link points to them.", "files"},
        {{"l", "toc-depth"}, "Number of levels of the table of contents which are shown.", "int"},
        {{"m", "min-delay"}, "Set minimum time per frame in milliseconds.\nThis is useful when using \\animation in LaTeX beamer.", "ms"},
        {{"M", "memory"}, "Maximum size of cache in MiB. A negative number is treated as infinity.", "int"},
        {{"o", "columns"}, "Number of columns in overview.", "int"},
        {{"p", "page-part"}, "Set half of the page to be the presentation, the other half to be the notes. Values are \"l\" or \"r\" for presentation on the left or right half of the page, respectively.\nIf the presentation was created with \"\\setbeameroption{show notes on second screen=right}\", you should use \"--page-part=right\".", "side"},
        {{"r", "renderer"}, "\"poppler\", \"custom\" or command: Command for rendering pdf pages to cached images. This command should write a png image to standard output using the arguments %file (path to file), %page (page number), %width and %height (image size in pixels).", "string"},
        {{"s", "scrollstep"}, "Number of pixels which represent a scroll step for a touch pad scroll signal.", "int"},
        {{"t", "timer"}, "Set timer to <time>.\nPossible formats are \"[m]m\", \"[m]m:ss\" and \"h:mm:ss\".", "time"},
        {{"u", "urlsplit"}, "Character which is used to split links into an url and arguments.", "char"},
        {{"v", "video-cache"}, "Preload videos for the following slide.", "bool"},
        {{"w", "pid2wid"}, "Program that converts a PID to a Window ID.", "file"},
    });
    parser.process(app);

    // set up a settings manager
    // This is only tested on GNU/Linux! I don't know whether it will work in Windows or MacOS
    #ifdef Q_OS_MAC
    // untested!
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
    #elif Q_OS_WIN
    // untested!
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
    #else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
    #endif

    // Create the GUI
    // w will be the object which manages everything.
    ControlScreen * w;

    // Handle positional arguments for the pdf files:
    switch (parser.positionalArguments().size()) {
        case 1:
            // If only a presentation file is given:
            w = new ControlScreen(parser.positionalArguments()[0]);
            break;
        case 2:
            // If a note file and a presentation file are given:
            w = new ControlScreen(parser.positionalArguments()[0], parser.positionalArguments()[1]);
            break;
        case 0:
            // No files given: Open files in a QFileDialog
            {
                QString presentationPath = QFileDialog::getOpenFileName(nullptr, "Open Slides", "", "Documents (*.pdf)");
                if (presentationPath.isEmpty()) {
                    qCritical() << "No presentation file specified";
                    parser.showHelp(1);
                }
                QString notesPath = "";
                // Check if the presentation file should include notes (beamer option show notes on second screen).
                // If the presenation file is interpreted as a normal presentation, open a note file in a QFileDialog.
                if (parser.value("p").isEmpty()) {
                    if (!settings.contains("page-part") || settings.value("page-part").toString()=="none" || settings.value("page-part").toString()=="0")
                        notesPath = QFileDialog::getOpenFileName(nullptr, "Open Notes", presentationPath, "Documents (*.pdf)");
                }
                else if (parser.value("p")=="none" || parser.value("p")=="0")
                    notesPath = QFileDialog::getOpenFileName(nullptr, "Open Notes", presentationPath, "Documents (*.pdf)");
                w = new ControlScreen(presentationPath, notesPath);
            }
            break;
        default:
            qCritical() << "Received more than 2 positional arguments.";
            parser.showHelp(1);
    }

    { // set colors
        QColor bgColor, textColor, presentationColor;
        // notes background color: background of the control screen
        if (settings.contains("notes background color"))
            bgColor = settings.value("notes background color").value<QColor>();
        else
            bgColor = Qt::gray;
        // notes text color: text color on the control screen
        if (settings.contains("notes text color"))
            textColor = settings.value("notes text color").value<QColor>();
        else
            textColor = Qt::black;
        // presentation color: background color on the presentation screen
        if (settings.contains("presentation color"))
            presentationColor = settings.value("presentation color").value<QColor>();
        else
            presentationColor = Qt::black;
        w->setColor(bgColor, textColor);
        w->setPresentationColor(presentationColor);
    }

    { // Set colors for presentation timer
        settings.beginGroup("timer");
        // All elements in the group timer should be points (time, color).
        QStringList keys = settings.childKeys();
        if (keys.isEmpty())
            w->sendTimerColors({-150,0,150}, {Qt::white, Qt::green, Qt::red});
        else {
            // Use a QMap to store the values, because it is automatically sorted by the keys.
            QMap<int,QColor> map;
            bool ok;
            int time;
            QColor color;
            for (QStringList::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                time = (*key_it).toInt(&ok);
                if (ok) {
                    color = QColor(settings.value(*key_it).toString());
                    if (color.isValid())
                        map[time] = color;
                    else
                        qCritical() << "Color" << settings.value(*key_it) << "not understood. Should be a color.";
                }
                else
                    qCritical() << "Time interval" << *key_it << "not understood. Should be an integer.";
            }
            w->sendTimerColors(map.keys(), map.values());
        }
        settings.endGroup();
    }


    { // Handle keyboard shortcuts
        settings.beginGroup("keys");
        QStringList keys = settings.childKeys();
        if (keys.isEmpty())
            w->setKeyMap(new QMap<int, QList<int>>(defaultKeyMap));
        else {
            for (QStringList::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                QKeySequence keySequence = QKeySequence(*key_it, QKeySequence::NativeText);
                const int key = keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3];
                if (key!=0) {
                    QStringList actions = settings.value(*key_it).toStringList();
                    for (QStringList::const_iterator action_it=actions.begin(); action_it!=actions.cend(); action_it++) {
                        const int action = keyActionMap.value(action_it->toLower(), -1);
                        if (action!=-1)
                            w->setKeyMapItem(key, action);
                        else
                            qCritical() << "Could not understand action" << *action_it << "for key" << *key_it;
                    }
                }
                else
                    qCritical() << "Could not understand key" << *key_it;
            }
        }
        settings.endGroup();
    }

    // Read the arguments in parser.
    // Each argument can have a default value in settings.

    // Split page if necessary (beamer option show notes on second screen)
    if (!parser.value("p").isEmpty()) {
        QString value = parser.value("p");
        if ( value == "r" || value == "right" )
            w->setPagePart(RightHalf);
        else if ( value == "l" || value == "left" )
            w->setPagePart(LeftHalf);
        else if (value != "none" && value != "0") {
            qCritical() << "option \"" << value << "\" to page-part not understood.";
            // Try to get a default option from the config file
            if (settings.contains("page-part")) {
                value = settings.value("page-part").toString();
                if ( value == "r" || value == "right" )
                    w->setPagePart(RightHalf);
                else if ( value == "l" || value == "left" )
                    w->setPagePart(LeftHalf);
                else if (value != "none" && value != "0")
                    qCritical() << "option \"" << value << "\" to page-part in config not understood.";
            }
        }
    }
    else if (settings.contains("page-part")) {
        QString value = settings.value("page-part").toString();
        if ( value == "r" || value == "right" )
            w->setPagePart(RightHalf);
        else if ( value == "l" || value == "left" )
            w->setPagePart(LeftHalf);
        else if (value != "none" && value != "0")
            qCritical() << "option \"" << value << "\" to page-part in config not understood.";
    }

    // Set presentation time
    if (!parser.value("t").isEmpty())
        emit w->sendTimerString(parser.value("t"));
    else if (settings.contains("timer"))
        emit w->sendTimerString(settings.value("timer").toStringList()[0]);

    // Set minimum time per frame
    if (!parser.value("m").isEmpty()) {
        bool success;
        int delay = parser.value("m").toInt(&success);
        if (success)
            emit w->sendAnimationDelay(delay);
        else {
            qCritical() << "option \"" << parser.value("m") << "\" to min-delay not understood.";
            // Try to get a default option from the config file
            if (settings.contains("min-delay")) {
                delay = settings.value("min-delay").toInt(&success);
                if (success)
                    emit w->sendAnimationDelay(delay);
                else
                    qCritical() << "option \"" << settings.value("min-delay") << "\" to min-delay in config not understood.";
            }
        }
    }
    else if (settings.contains("min-delay")) {
        bool success;
        int delay = settings.value("min-delay").toInt(&success);
        if (success)
            emit w->sendAnimationDelay(delay);
        else
            qCritical() << "option \"" << settings.value("min-delay") << "\" to min-delay in config not understood.";
    }

    // Set autostart or delay for multimedia content
    if (!parser.value("a").isEmpty()) {
        double delay;
        bool success;
        QString a = parser.value("a").toLower();
        delay = a.toDouble(&success);
        if (success)
            emit w->sendAutostartDelay(delay);
        else if (a == "true")
            emit w->sendAutostartDelay(0.);
        else if (a == "false")
            emit w->sendAutostartDelay(-2.);
        else {
            qCritical() << "option \"" << parser.value("a") << "\" to autoplay not understood.";
            // Try to get a default option from the config file
            if (settings.contains("autoplay")) {
                a = settings.value("autoplay").toString().toLower();
                delay = a.toDouble(&success);
                if (success)
                    emit w->sendAutostartDelay(delay);
                else if (a == "true")
                    emit w->sendAutostartDelay(0.);
                else if (a == "false")
                    emit w->sendAutostartDelay(-2.);
                else
                    qCritical() << "option \"" << settings.value("autoplay") << "\" to autoplay in config not understood.";
            }
        }
    }
    else if (settings.contains("autoplay")) {
        double delay;
        bool success;
        QString a = settings.value("autoplay").toString().toLower();
        delay = a.toDouble(&success);
        if (success)
            emit w->sendAutostartDelay(delay);
        else if (a == "true")
            emit w->sendAutostartDelay(0.);
        else if (a == "false")
            emit w->sendAutostartDelay(-2.);
        else
            qCritical() << "option \"" << settings.value("autoplay") << "\" to autoplay in config not understood.";
    }

    // Set number of columns in overview
    if (!parser.value("o").isEmpty()) {
        bool success;
        int columns = parser.value("o").toInt(&success);
        if (success)
            w->setOverviewColumns(columns);
        else {
            qCritical() << "option \"" << parser.value("o") << "\" to columns not understood.";
            // Try to get a default option from the config file
            if (settings.contains("min-delay")) {
                columns = settings.value("min-delay").toInt(&success);
                if (success)
                    w->setOverviewColumns(columns);
                else
                    qCritical() << "option \"" << settings.value("columns") << "\" to columns in config not understood.";
            }
        }
    }
    else if (settings.contains("columns")) {
        bool success;
        int columns = settings.value("columns").toInt(&success);
        if (success)
            w->setOverviewColumns(columns);
        else
            qCritical() << "option \"" << settings.value("columns") << "\" to columns in config not understood.";
    }

    // Set autostart or delay for embedded applications
    if (!parser.value("A").isEmpty()) {
        double delay;
        bool success;
        QString a = parser.value("A").toLower();
        delay = a.toDouble(&success);
        if (success)
            emit w->sendAutostartEmbeddedDelay(delay);
        else if (a == "true")
            emit w->sendAutostartEmbeddedDelay(0.);
        else if (a == "false")
            emit w->sendAutostartEmbeddedDelay(-2.);
        else {
            qCritical() << "option \"" << parser.value("A") << "\" to autoplay not understood.";
            // Try to get a default option from the config file
            if (settings.contains("autostart-emb")) {
                a = settings.value("autostart-emb").toString().toLower();
                delay = a.toDouble(&success);
                if (success)
                    emit w->sendAutostartEmbeddedDelay(delay);
                else if (a == "true")
                    emit w->sendAutostartEmbeddedDelay(0.);
                else if (a == "false")
                    emit w->sendAutostartEmbeddedDelay(-2.);
                else
                    qCritical() << "option \"" << settings.value("autostart-emb") << "\" to autoplay in config not understood.";
            }
        }
    }
    else if (settings.contains("autostart-emb")) {
        double delay;
        bool success;
        QString a = settings.value("autostart-emb").toString().toLower();
        delay = a.toDouble(&success);
        if (success)
            emit w->sendAutostartEmbeddedDelay(delay);
        else if (a == "true")
            emit w->sendAutostartEmbeddedDelay(0.);
        else if (a == "false")
            emit w->sendAutostartEmbeddedDelay(-2.);
        else
            qCritical() << "option \"" << settings.value("autostart-emb") << "\" to autoplay in config not understood.";
    }

    // Set files, which will be executed in an embedded widget
    // TODO: Wildcard characters
    if (!parser.value("e").isEmpty()) {
        const QStringList files = parser.value("e").split(",");
        w->setEmbedFileList(files);
    }
    else if (settings.contains("embed")) {
        const QStringList files = settings.value("embed").toStringList();
        w->setEmbedFileList(files);
    }

    // Set program, which will convert PIDs to Window IDs
    if (!parser.value("w").isEmpty()) {
        if (parser.value("w").toLower() != "none")
            w->setPid2WidConverter(parser.value("w"));
    }
    else if (settings.contains("pid2wid")) {
        QString string = settings.value("pid2wid").toString();
        if (string.toLower() != "none")
            w->setPid2WidConverter(string);
    }

    // Set character, which is used to split links into a file name and arguments
    if (!parser.value("u").isEmpty())
        w->setUrlSplitCharacter(parser.value("u"));
    else if (settings.contains("urlsplit"))
        w->setUrlSplitCharacter(settings.value("urlsplit").toString());

    // Set scroll step for touch pad input devices
    if (!parser.value("s").isEmpty()) {
        bool success;
        int step = parser.value("s").toInt(&success);
        if (success)
            w->setScrollDelta(step);
        else {
            qCritical() << "option \"" << parser.value("s") << "\" to scrollstep not understood.";
            // Try to get a default option from the config file
            if (settings.contains("scrollstep")) {
                step = settings.value("scrollstep").toInt(&success);
                if (success)
                    w->setScrollDelta(step);
                else
                    qCritical() << "option \"" << settings.value("scrollstep") << "\" to scrollstep in config not understood.";
            }
        }
    }
    else if (settings.contains("scrollstep")) {
        bool success;
        int step = settings.value("scrollstep").toInt(&success);
        if (success)
            w->setScrollDelta(step);
        else
            qCritical() << "option \"" << settings.value("scrollstep") << "\" to scrollstep in config not understood.";
    }

    // Set maximum number of cached slides
    if (!parser.value("c").isEmpty()) {
        bool success;
        int num = parser.value("c").toInt(&success);
        if (success)
            w->setCacheNumber(num);
        else {
            qCritical() << "option \"" << parser.value("c") << "\" to cache not understood.";
            // Try to get a default option from the config file
            if (settings.contains("cache")) {
                num = settings.value("cache").toInt(&success);
                if (success)
                    w->setCacheNumber(num);
                else
                    qCritical() << "option \"" << settings.value("cache") << "\" to cache in config not understood.";
            }
        }
    }
    else if (settings.contains("cache")) {
        bool success;
        int num = settings.value("cache").toInt(&success);
        if (success)
            w->setCacheNumber(num);
        else
            qCritical() << "option \"" << settings.value("cache") << "\" to cache in config not understood.";
    }

    // Set maximum cache size
    if (!parser.value("M").isEmpty()) {
        bool success;
        int size = parser.value("M").toInt(&success);
        if (success)
            w->setCacheSize(1048576L * size);
        else {
            qCritical() << "option \"" << parser.value("M") << "\" to memory not understood.";
            // Try to get a default option from the config file
            if (settings.contains("memory")) {
                size = settings.value("memory").toInt(&success);
                if (success)
                    w->setCacheSize(1048576L * size);
                else
                    qCritical() << "option \"" << settings.value("memory") << "\" to memory in config not understood.";
            }
        }
    }
    else if (settings.contains("memory")) {
        bool success;
        int size = settings.value("memory").toInt(&success);
        if (success)
            w->setCacheSize(1048576L * size);
        else
            qCritical() << "option \"" << settings.value("memory") << "\" to memory in config not understood.";
    }

    // Set number of visible TOC levels
    if (!parser.value("l").isEmpty()) {
        bool success;
        int num = parser.value("l").toInt(&success);
        if (success)
            w->setTocLevel(num);
        else {
            qCritical() << "option \"" << parser.value("l") << "\" to toc-depth not understood.";
            // Try to get a default option from the config file
            if (settings.contains("toc-depth")) {
                num = settings.value("toc-depth").toInt(&success);
                if (success)
                    w->setTocLevel(num);
                else
                    qCritical() << "option \"" << settings.value("toc-depth") << "\" to toc-depth in config not understood.";
            }
        }
    }
    else if (settings.contains("toc-depth")) {
        bool success;
        int num = settings.value("toc-depth").toInt(&success);
        if (success)
            w->setTocLevel(num);
        else
            qCritical() << "option \"" << settings.value("toc-depth") << "\" to toc-depth in config not understood.";
    }

    // Set custom renderer
    if (!parser.value("r").isEmpty()) {
        if (parser.value("r") == "custom") {
            if (settings.contains("renderer"))
                w->setRenderer(settings.value("renderer").toString().split(" "));
            else
                qCritical() << "Ignored request to use undefined custom renderer";
        }
        else if (parser.value("r") != "poppler") {
            if (!w->setRenderer(parser.value("r").split(" ")) && settings.contains("renderer"))
                w->setRenderer(settings.value("renderer").toString().split(" "));
        }

    }
    else if (settings.contains("renderer"))
        w->setRenderer(settings.value("renderer").toString().split(" "));

    // Set video cache
    if (!parser.value("v").isEmpty()) {
        QString value = parser.value("v").toLower();
        if (value == "true")
            w->setCacheVideos(true);
        else if (value == "false")
            w->setCacheVideos(false);
        else if (settings.contains("video-cache")) {
            qCritical() << "option \"" << parser.value("v") << "\" to video-cache not understood.";
            value = settings.value("video-cache").toString().toLower();
            if (value == "true")
                w->setCacheVideos(true);
            else if (value == "false")
                w->setCacheVideos(false);
            else
                qCritical() << "option \"" << settings.value("video-cache") << "\" to video-cache in config not understood.";
        }
    }
    else if (settings.contains("video-cache")) {
        QString value = settings.value("video-cache").toString().toLower();
        if (value == "true")
            w->setCacheVideos(true);
        else if (value == "false")
            w->setCacheVideos(false);
        else
            qCritical() << "option \"" << settings.value("video-cache") << "\" to video-cache in config not understood.";
    }


    // show the GUI
    w->show();
    w->activateWindow();

    // Render first page on presentation screen
    emit w->sendNewPageNumber(0);
    // Render first page on control screen
    w->renderPage(0);

    // Here one could update the cache.
    // But you probably first want to adjust the window size and then update it with key shortcut c.
    //w->updateCache();

    // start the execution loop
    int status = app.exec();
    delete w;
    return status;
}
