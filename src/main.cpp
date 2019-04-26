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
#include <QJsonObject>
#include <QJsonDocument>
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

double doubleFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, double const def)
{
    // Handle arguments that are either double or bool
    bool ok;
    double result;
    QString value = parser.value(name).toLower();
    if (!value.isEmpty()) {
        result = value.toDouble(&ok);
        if (ok)
            return result;
        else if (value == "true")
            return 0.;
        else if (value == "false")
            return -2.;
        else
            qCritical() << "option \"" << parser.value(name) << "\" to" << name << "not understood. Should be a number.";
    }
    if (local.contains(name)) {
        result = local.value(name).toDouble(&ok);
        if (ok)
            return result;
        else {
            value = local.value(name).toString().toLower();
            if (value == "true")
                return 0.;
            else if (value == "false")
                return -2.;
            else
                qCritical() << "option \"" << settings.value(name) << "\" to" << name << "in local config not understood. Should be a number.";
        }
    }
    if (settings.contains(name)) {
        result = settings.value(name).toDouble(&ok);
        if (ok)
            return result;
        else {
            value = settings.value(name).toString().toLower();
            if (value == "true")
                return 0.;
            else if (value == "false")
                return -2.;
            else
                qCritical() << "option \"" << settings.value(name) << "\" to" << name << "in config not understood. Should be a number.";
        }
    }
    return def;
}

int intFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, int const def)
{
    bool ok;
    int result;
    if (!parser.value(name).isEmpty()) {
        result = parser.value(name).toInt(&ok);
        if (ok)
            return result;
        else
            qCritical() << "option \"" << parser.value(name) << "\" to" << name << "not understood. Should be an integer.";
    }
    if (local.contains(name)) {
        result = local.value(name).toInt(&ok);
        if (ok)
            return result;
        else
            qCritical() << "option \"" << settings.value(name) << "\" to" << name << "in local config not understood. Should be an integer.";
    }
    if (settings.contains(name)) {
        result = settings.value(name).toInt(&ok);
        if (ok)
            return result;
        else
            qCritical() << "option \"" << settings.value(name) << "\" to" << name << "in config not understood. Should be an integer.";
    }
    return def;
}


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
        {{"t", "time"}, "Set presentation time.\nPossible formats are \"[m]m\", \"[m]m:ss\" and \"h:mm:ss\".", "time"},
        {{"u", "urlsplit"}, "Character which is used to split links into an url and arguments.", "char"},
        {{"v", "video-cache"}, "Preload videos for the following slide.", "bool"},
        {{"w", "pid2wid"}, "Program that converts a PID to a Window ID.", "file"},
        {{"j", "json"}, "Local JSON configuration file.", "file"},
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

    // Load an optional local configuration file
    // This file can be used to set e.g. times per slide.
    QVariantMap local = QVariantMap();
    if (!parser.value("j").isEmpty()) {
        QFile jsonFile(parser.value("j"));
        if (jsonFile.exists()) {
            jsonFile.open(QFile::ReadOnly);
            QJsonParseError* error = nullptr;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), error);
            if (jsonDoc.isNull() || jsonDoc.isEmpty()) {
                qCritical() << "Failed to load local configuration file: File is empty or parsing JSON failed";
                if (error != nullptr)
                    qInfo() << "Reported error:" << error->errorString();
            }
            else
                local = jsonDoc.object().toVariantMap();
        }
        else
            qCritical() << "Failed to load local configuration file: File doesn't exist.";
    }

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
        if (local.contains("presentation")) {
            if (local.contains("notes"))
                w = new ControlScreen(local.value("presentation").toString(), local.value("notes").toString());
            else
                w = new ControlScreen(local.value("presentation").toString());
        }
        else {
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
        if (local.contains("notes background color"))
            bgColor = local.value("notes background color").value<QColor>();
        else if (settings.contains("notes background color"))
            bgColor = settings.value("notes background color").value<QColor>();
        else
            bgColor = Qt::gray;
        // notes text color: text color on the control screen
        if (local.contains("notes text color"))
            textColor = local.value("notes text color").value<QColor>();
        else if (settings.contains("notes text color"))
            textColor = settings.value("notes text color").value<QColor>();
        else
            textColor = Qt::black;
        // presentation color: background color on the presentation screen
        if (local.contains("presentation color"))
            presentationColor = local.value("presentation color").value<QColor>();
        else if (settings.contains("presentation color"))
            presentationColor = settings.value("presentation color").value<QColor>();
        else
            presentationColor = Qt::black;
        w->setColor(bgColor, textColor);
        w->setPresentationColor(presentationColor);
    }

    // Handle array / map arguments

    if (local.contains("timer")) { // Set colors for presentation timer
        QVariantMap variantMap = local["timer"].value<QVariantMap>();
        QList<QString> keys = variantMap.keys();
        if (keys.isEmpty())
            w->sendTimerColors({-150,0,150}, {Qt::white, Qt::green, Qt::red});
        else {
            // Use a QMap to store the values, because it is automatically sorted by the keys.
            QMap<int,QColor> map;
            bool ok;
            int time;
            QColor color;
            for (QList<QString>::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                time = (*key_it).toInt(&ok);
                if (ok) {
                    color = variantMap[*key_it].value<QColor>();
                    if (color.isValid())
                        map[time] = color;
                    else
                        qCritical() << "Color" << variantMap[*key_it].toString() << "not understood. Should be a color.";
                }
                else
                    qCritical() << "Time interval" << *key_it << "not understood. Should be an integer.";
            }
            w->sendTimerColors(map.keys(), map.values());
        }
    }
    else { // Set colors for presentation timer
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
                    color = settings.value(*key_it).value<QColor>();
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


    if (local.contains("keys")) { // Handle key bindings
        QVariantMap variantMap = local["keys"].value<QVariantMap>();
        QList<QString> keys = variantMap.keys();
        if (keys.isEmpty())
            w->setKeyMap(new QMap<int, QList<int>>(defaultKeyMap));
        else {
            for (QList<QString>::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                QKeySequence keySequence = QKeySequence(*key_it, QKeySequence::NativeText);
                const int key = keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3];
                if (key!=0) {
                    QStringList actions = variantMap[*key_it].toStringList();
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
    }
    else { // Handle key bindings
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

    if (local.contains("page times")) { // set times per slide for timer color change
        QMap<int, QTime> map;
        QVariantMap variantMap = local["page times"].value<QVariantMap>();
        bool ok;
        int key, min, sec;
        QStringList timeSplit;
        for (QVariantMap::const_iterator it=variantMap.cbegin(); it!=variantMap.cend(); it++) {
            key = it.key().toInt(&ok);
            if (!ok) {
                qCritical() << "In local config / page times: Did not understand slide number" << it.key();
                continue;
            }
            timeSplit = it->toString().split(":");
            if (timeSplit.length()==2) {
                min = timeSplit[0].toInt(&ok);
                if (ok) {
                    sec = timeSplit[1].toInt(&ok);
                    if (ok) {
                        map.insert(key, QTime(0, min, sec));
                        continue;
                    }
                }
            }
            qCritical() << "In local config / page times: Did not understand time" << *it << "Required format: \"mm:ss\"";
            continue;
        }
        w->setTimerMap(map);
    }

    { // Handle more complicated options
        // Split page if necessary (beamer option show notes on second screen)
        bool found = false;
        if (!parser.value("p").isEmpty()) {
            QString value = parser.value("p");
            if ( value == "r" || value == "right" ) {
                w->setPagePart(RightHalf);
                found = true;
            }
            else if ( value == "l" || value == "left" ) {
                w->setPagePart(LeftHalf);
                found = true;
            }
            else if (value != "none" && value != "0")
                qCritical() << "option \"" << value << "\" to page-part not understood.";
        }
        if (!found && local.contains("page-part")) {
            QString value = settings.value("page-part").toString();
            if ( value == "r" || value == "right" ) {
                w->setPagePart(RightHalf);
                found = true;
            }
            else if ( value == "l" || value == "left" ) {
                w->setPagePart(LeftHalf);
                found = true;
            }
            else if (value != "none" && value != "0")
                qCritical() << "option \"" << value << "\" to page-part in local config not understood.";
        }
        if (!found && settings.contains("page-part")) {
                QString value = settings.value("page-part").toString();
                if ( value == "r" || value == "right" )
                    w->setPagePart(RightHalf);
                else if ( value == "l" || value == "left" )
                    w->setPagePart(LeftHalf);
                else if (value != "none" && value != "0")
                    qCritical() << "option \"" << value << "\" to page-part in config not understood.";
        }

        // Set video cache
        if (!parser.value("v").isEmpty()) {
            QString value = parser.value("v").toLower();
            if (value == "true") {
                w->setCacheVideos(true);
                found = true;
            }
            else if (value == "false") {
                w->setCacheVideos(false);
                found = true;
            }
            else
                qCritical() << "option \"" << parser.value("v") << "\" to video-cache not understood.";
        }
        if (!found && local.contains("video-cache")) {
            QString value = settings.value("video-cache").toString().toLower();
            if (value == "true") {
                w->setCacheVideos(true);
                found = true;
            }
            else if (value == "false") {
                w->setCacheVideos(false);
                found = true;
            }
            else
                qCritical() << "option \"" << local.value("video-cache") << "\" to video-cache in local config not understood.";
        }
        if (!found && settings.contains("video-cache")) {
            QString value = settings.value("video-cache").toString().toLower();
            if (value == "true")
                w->setCacheVideos(true);
            else if (value == "false")
                w->setCacheVideos(false);
            else
                qCritical() << "option \"" << settings.value("video-cache") << "\" to video-cache in config not understood.";
        }
    }

    // Simple options

    // Set files, which will be executed in an embedded widget
    // TODO: Wildcard characters
    if (!parser.value("e").isEmpty())
        w->setEmbedFileList(parser.value("e").split(","));
    else if (local.contains("embed"))
        w->setEmbedFileList(local.value("embed").toStringList());
    else if (settings.contains("embed"))
        w->setEmbedFileList(settings.value("embed").toStringList());

    // Set program, which will convert PIDs to Window IDs
    if (!parser.value("w").isEmpty()) {
        if (parser.value("w").toLower() != "none")
            w->setPid2WidConverter(parser.value("w"));
    }
    else if (local.contains("pid2wid")) {
        QString string = local.value("pid2wid").toString();
        if (string.toLower() != "none")
            w->setPid2WidConverter(string);
    }
    else if (settings.contains("pid2wid")) {
        QString string = settings.value("pid2wid").toString();
        if (string.toLower() != "none")
            w->setPid2WidConverter(string);
    }

    // Set character, which is used to split links into a file name and arguments
    if (!parser.value("u").isEmpty())
        w->setUrlSplitCharacter(parser.value("u"));
    else if (local.contains("urlsplit"))
        w->setUrlSplitCharacter(local.value("urlsplit").toString());
    else if (settings.contains("urlsplit"))
        w->setUrlSplitCharacter(settings.value("urlsplit").toString());


    { // Handle settings that are either double or bool
        // Set autostart or delay for multimedia content
        double value;
        value = doubleFromConfig(parser, local, settings, "autoplay", -2.);
        emit w->sendAutostartDelay(value);

        // Set autostart or delay for embedded applications
        value = doubleFromConfig(parser, local, settings, "autostart-emb", -2.);
        emit w->sendAutostartEmbeddedDelay(value);
    }

    { // Settings with integer values
        int value;

        // Set minimum time per frame
        value = intFromConfig(parser, local, settings, "min-delay", 40);
        emit w->sendAnimationDelay(value);

        // Set number of columns in overview
        value = intFromConfig(parser, local, settings, "columns", 5);
        if (value<1)
            qCritical() << "You tried to set a number of columns < 1. You shouldn't expect anyting reasonable!";
        else
            w->setOverviewColumns(value);

        // Set scroll step for touch pad input devices
        value = intFromConfig(parser, local, settings, "scrollstep", 200);
        w->setScrollDelta(value);

        // Set maximum number of cached slides
        value = intFromConfig(parser, local, settings, "cache", -1);
        w->setCacheNumber(value);

        // Set maximum cache size
        value = intFromConfig(parser, local, settings, "memory", 100);
        w->setCacheSize(1048576L * value);

        // Set number of visible TOC levels
        value = intFromConfig(parser, local, settings, "toc-depth", 2);
        if (value<1)
            qCritical() << "You tried to set a number of TOC levels < 1. You shouldn't expect anyting reasonable!";
        else
        w->setTocLevel(value);
    }


    // Settings, which can cause exceptions

    // Set presentation time
    try {
        if (parser.value("t").isEmpty())
            throw 0;
        emit w->sendTimerString(parser.value("t"));
    }
    catch (int const err) {
        try {
            if (!local.contains("time"))
                throw 0;
            emit w->sendTimerString(local.value("time").toStringList()[0]);
        }
        catch (int const err) {
            if (settings.contains("time"))
                emit w->sendTimerString(settings.value("time").toStringList()[0]);
        }
    }

    // Set custom renderer
    try {
        if (parser.value("r").isEmpty())
            throw 0;
        if (parser.value("r") == "custom")
            throw -1;
        if (parser.value("r") != "poppler")
            w->setRenderer(parser.value("r").split(" "));
    }
    catch (int const err) {
        if (err > 0)
            qCritical() << "Failed to set custom renderer";
        try {
            if (!local.contains("renderer"))
                throw err>0 ? 0 : err;
            w->setRenderer(local.value("renderer").toString().split(" "));
        }
        catch (int const err) {
            if (err > 0)
                qCritical() << "Failed to set custom renderer from local configuration file";
            if (settings.contains("renderer")) {
                try {
                    w->setRenderer(settings.value("renderer").toString().split(" "));
                }
                catch (int const err) {
                    qCritical() << "Failed to set custom renderer from configuration";
                }
            }
            if (err==-1)
                qCritical() << "Ignored request to use undefined custom renderer";
        }
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
