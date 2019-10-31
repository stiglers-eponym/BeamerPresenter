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

#include <QSettings>
#include <QApplication>
#include <QCommandLineParser>
#include <QKeySequence>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMimeDatabase>
#include "controlscreen.h"

static const QMap<QString, KeyAction> keyActionMap {
    {"previous", KeyAction::Previous},
    {"next", KeyAction::Next},
    {"previous notes", KeyAction::PreviousNotes},
    {"next notes", KeyAction::NextNotes},
    {"previous skipping overlays", KeyAction::PreviousSkippingOverlays},
    {"next skipping overlays", KeyAction::NextSkippingOverlays},
    {"previous notes skipping overlays", KeyAction::PreviousNotesSkippingOverlays},
    {"next notes skipping overlays", KeyAction::NextNotesSkippingOverlays},
    {"go to", KeyAction::GoToPage},
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
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {"start embedded current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current page", KeyAction::StartEmbeddedCurrentSlide},
    {"start embedded applications current slide", KeyAction::StartEmbeddedCurrentSlide},
    {"start all embedded", KeyAction::StartAllEmbedded},
    {"start all embedded applications", KeyAction::StartAllEmbedded},
    {"close all embedded", KeyAction::CloseAllEmbedded},
    {"close all embedded applications", KeyAction::CloseAllEmbedded},
    {"close embedded current page", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded current slide", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded applications current page", KeyAction::CloseEmbeddedCurrentSlide},
    {"close embedded applications current slide", KeyAction::CloseEmbeddedCurrentSlide},
#endif
    {"play multimedia", KeyAction::PlayMultimedia},
    {"pause multimedia", KeyAction::PauseMultimedia},
    {"play pause multimedia", KeyAction::PlayPauseMultimedia},
    {"toggle multimedia", KeyAction::PlayPauseMultimedia},

    {"toggle timer", KeyAction::PlayPauseTimer},
    {"pause timer", KeyAction::PauseTimer},
    {"continue timer", KeyAction::ContinueTimer},
    {"reset timer", KeyAction::ResetTimer},
    {"toggle toc", KeyAction::ToggleTOC},
    {"show toc", KeyAction::ShowTOC},
    {"hide toc", KeyAction::HideTOC},
    {"hide overview", KeyAction::HideOverview},
    {"show overview", KeyAction::ShowOverview},
    {"toggle overview", KeyAction::ToggleOverview},
    {"toggle cursor", KeyAction::ToggleCursor},
    {"show cursor", KeyAction::ShowCursor},
    {"hide cursor", KeyAction::HideCursor},
    {"full screen", KeyAction::FullScreen},
    {"reload", KeyAction::Reload},
    {"quit", KeyAction::Quit},

    {"clear annotations", KeyAction::ClearAnnotations},
    {"hand tool", KeyAction::DrawNone},
    {"none", KeyAction::DrawNone},
    {"pointer", KeyAction::DrawPointer},
    {"pen", KeyAction::DrawPen},
    {"highlighter", KeyAction::DrawHighlighter},
    {"torch", KeyAction::DrawTorch},
    {"magnifier", KeyAction::DrawMagnifier},
    {"eraser", KeyAction::DrawEraser},

    {"toggle draw mode", KeyAction::ToggleDrawMode},
    {"enter draw mode", KeyAction::DrawMode},
    {"draw mode", KeyAction::DrawMode},
    {"hide draw slide", KeyAction::HideDrawSlide},
    {"end draw mode", KeyAction::HideDrawSlide},
    {"end drawing", KeyAction::HideDrawSlide},
    {"save drawings", KeyAction::SaveDrawings},
    {"load drawings", KeyAction::LoadDrawings},
};

static const QMap<QString, DrawTool> toolMap {
    {"pen", DrawTool::Pen},
    {"highlighter", DrawTool::Highlighter},
    {"none", DrawTool::NoTool},
    {"pointer", DrawTool::Pointer},
    {"eraser", DrawTool::Eraser},
    {"torch", DrawTool::Torch},
    {"magnifier", DrawTool::Magnifier},
};

static const QMap<quint32, QList<KeyAction>> defaultKeyMap = {
    {Qt::Key_PageUp, {KeyAction::Previous}},
    {Qt::Key_PageDown, {KeyAction::Next}},
    {Qt::Key_Left, {KeyAction::Previous}},
    {Qt::Key_Right, {KeyAction::Next}},
    {Qt::Key_Up, {KeyAction::PreviousSkippingOverlays}},
    {Qt::Key_Down, {KeyAction::NextSkippingOverlays}},
    {Qt::Key_G, {KeyAction::GoToPage}},
    {Qt::Key_End, {KeyAction::LastPage}},
    {Qt::Key_Home, {KeyAction::FirstPage}},
    {Qt::Key_Return, {KeyAction::SyncFromControlScreen}},
    {Qt::Key_Escape, {KeyAction::SyncFromPresentationScreen, KeyAction::HideTOC, KeyAction::HideOverview, KeyAction::HideDrawSlide}},
    {Qt::Key_Space, {KeyAction::Update}},

    {Qt::Key_C, {KeyAction::UpdateCache}},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
    {Qt::Key_E, {KeyAction::StartEmbeddedCurrentSlide}},
    {Qt::Key_E+Qt::ShiftModifier, {KeyAction::StartAllEmbedded}},
#endif
    {Qt::Key_M, {KeyAction::PlayPauseMultimedia}},

    {Qt::Key_P, {KeyAction::PlayPauseTimer}},
    {Qt::Key_R, {KeyAction::ResetTimer}},
    {Qt::Key_T, {KeyAction::ShowTOC}},
    {Qt::Key_S, {KeyAction::ShowOverview}},
    {Qt::Key_O, {KeyAction::ToggleCursor}},
    {Qt::Key_F, {KeyAction::FullScreen}},
    {Qt::Key_U, {KeyAction::Reload}},
    {Qt::Key_Q+Qt::CTRL, {KeyAction::Quit}}
};

// Default configuration for tool selector (buttons)
static const QMap<quint8, QList<KeyAction>> defaultActionMap {
    {0, {KeyAction::ToggleDrawMode}},
    {1, {KeyAction::DrawEraser}},
    {2, {KeyAction::DrawPen}},
    {3, {KeyAction::DrawPen}},
    {4, {KeyAction::DrawHighlighter}},
    {16, {KeyAction::PlayMultimedia}},
    {17, {KeyAction::ClearAnnotations}},
    {18, {KeyAction::DrawPointer}},
    {19, {KeyAction::DrawTorch}},
    {20, {KeyAction::DrawMagnifier}},
};
static const QMap<quint8, QColor> defaultColorMap {
    {2, QColor("red")},
    {3, QColor("green")},
    {4, QColor(255,255,0,191)},
    {18, QColor(255,0,0,191)},
};

double doubleFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, double const def)
{
    // Handle arguments that are either double or bool
    bool ok;
    double result;
    QString value;
    if (parser.isSet(name))
        value = parser.value(name).toLower();
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

template <class T>
T intFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, T const def)
{
    bool ok;
    bool const is_signed = T(-1) < 0;
    T result;
    if (parser.isSet(name) && !parser.value(name).isEmpty()) {
        if (is_signed)
            result = T(parser.value(name).toInt(&ok));
        else
            result = T(parser.value(name).toUInt(&ok));
        if (ok)
            return result;
        else
            qCritical() << "option \"" << parser.value(name) << "\" to" << name << "not understood. Should be an integer.";
    }
    if (local.contains(name)) {
        if (is_signed)
            result = T(local.value(name).toInt(&ok));
        else
            result = T(local.value(name).toUInt(&ok));
        if (ok)
            return result;
        else
            qCritical() << "option \"" << settings.value(name) << "\" to" << name << "in local config not understood. Should be an integer.";
    }
    if (settings.contains(name)) {
        if (is_signed)
            result = T(settings.value(name).toInt(&ok));
        else
            result = T(settings.value(name).toUInt(&ok));
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
#ifdef QT_DEBUG
    app.setApplicationVersion("0.1.0+debugging, based on poppler " POPPLER_VERSION " and Qt " QT_VERSION_STR);
#else
    app.setApplicationVersion("0.1.0 based on poppler " POPPLER_VERSION " and Qt " QT_VERSION_STR);
#endif

    // set up command line argument parser
    QCommandLineParser parser;
    parser.setApplicationDescription( // TODO: keep this up to date.
            "\nSimple dual screen pdf presentation software.\n"
            "Default shortcuts:\n"
            "  c                Update cache\n"
#ifdef EMBEDDED_APPLICATIONS_ENABLED
            "  e                Start all embedded applications on the current slide\n"
            "  E                Start all embedded applications on all slides\n"
#endif
            "  g                Go to page (set focus to page number edit)\n"
            "  m                Play or pause all multimedia content\n"
            "  o                Toggle cursor visbility (only on presentation screen)\n"
            "  p                Pause / continue timer\n"
            "  Ctrl + q         Quit\n"
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
#ifdef EMBEDDED_APPLICATIONS_ENABLED
        {{"A", "autostart-emb"}, "true, false or number: Start embedded applications when entering a slide.\nA number is interpreted as a delay in seconds, after which applications are started.", "value"},
#endif
        {{"b", "blinds"}, "Number of blinds in binds slide transition", "int"},
        {{"c", "cache"}, "Number of slides that will be cached. A negative number is treated as infinity.", "int"},
        {{"d", "no-transitions"}, "Disable slide transitions."},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
        {{"e", "embed"}, "file1,file2,... Mark these files for embedding if an execution link points to them.", "files"},
#endif
        {{"j", "json"}, "Local JSON configuration file.", "file"},
        {{"l", "toc-depth"}, "Number of levels of the table of contents which are shown.", "int"},
        {{"m", "min-delay"}, "Set minimum time per frame in milliseconds.\nThis is useful when using \\animation in LaTeX beamer.", "ms"},
        {{"M", "memory"}, "Maximum size of cache in MiB. A negative number is treated as infinity.", "int"},
        {{"n", "no-notes"}, "Show only presentation and no notes."},
        {{"o", "columns"}, "Number of columns in overview.", "int"},
        {{"p", "page-part"}, "Set half of the page to be the presentation, the other half to be the notes. Values are \"l\" or \"r\" for presentation on the left or right half of the page, respectively.\nIf the presentation was created with \"\\setbeameroption{show notes on second screen=right}\", you should use \"--page-part=right\".", "side"},
        {{"r", "renderer"}, "\"poppler\", \"custom\" or command: Command for rendering pdf pages to cached images. This command should write a png image to standard output using the arguments %file (path to file), %page (page number), %width and %height (image size in pixels).", "string"},
        {{"s", "scrollstep"}, "Number of pixels which represent a scroll step for a touch pad scroll signal.", "int"},
        {{"t", "time"}, "Set presentation time.\nPossible formats are \"[m]m\", \"[m]m:ss\" and \"h:mm:ss\".", "time"},
        {{"u", "urlsplit"}, "Character which is used to split links into an url and arguments.", "char"},
        {{"v", "video-cache"}, "Preload videos for the following slide.", "bool"},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
        {{"w", "pid2wid"}, "Program that converts a PID to a Window ID.", "file"},
#endif
        {"force-show", "Force showing notes or presentation (if in a framebuffer) independent of QPA platform plugin."},
        {"force-touchpad", "Treat every scroll input as touch pad."},
        {"magnifier-size", "Radius of magnifier.", "pixels"},
        {"pointer-size", "Radius of magnifier.", "pixels"},
        {"torch-size", "Radius of torch.", "pixels"},
        {"highlighter-width", "Line width of highlighter.", "pixels"},
        {"pen-width", "Line width of pens.", "pixels"},
        {"eraser-size", "Radius of eraser.", "pixels"},
        {"icon-path", "Set path for default icons, e.g. /usr/share/icons/default", "path"},
    });
    parser.process(app);

    // Set up a settings manager
    // This is basically designed for GNU/Linux, where it loads $dir/beamerpresenter/beamerpresenter.conf for dir in $XDG_CONFIG_DIRS.
    // On MS Windows this (probably) tries to load a file USER_HOME\AppData\Roaming\beamerpresenter.ini or USER_HOME\AppData\Roaming\beamerpresenter\beamerpresenter.ini.
    // I don't know what happens on a Mac.
#ifdef Q_OS_MAC
    // untested!
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#else
#ifdef Q_OS_WIN
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#else
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#endif
#endif
    qDebug() << "Loaded settings:" << settings.allKeys();

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
        {
        if (!QFileInfo(parser.positionalArguments()[0]).exists()) {
            qCritical() << "File" << parser.positionalArguments()[0] << "does not exist!";
            return 1;
        }
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForFile(parser.positionalArguments()[0], QMimeDatabase::MatchContent);
        if (!type.isValid()) {
            type = db.mimeTypeForFile(parser.positionalArguments()[0], QMimeDatabase::MatchExtension);
            if (!type.isValid()) {
                qCritical() << "Did not understand type of file" << parser.positionalArguments()[0];
                return 1;
            }
        }
        if (type.suffixes().contains("pdf", Qt::CaseInsensitive)) {
            // If only a presentation file is given:
            w = new ControlScreen(parser.positionalArguments()[0]);
            break;
        }
        else if (type.suffixes().contains("txt") || type.suffixes().contains("json")) {
            // If a config file is given:
            QFile jsonFile(parser.positionalArguments()[0]);
            jsonFile.open(QFile::ReadOnly);
            QJsonParseError* error = nullptr;
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), error);
            if (jsonDoc.isNull() || jsonDoc.isEmpty()) {
                qCritical() << "Failed to load local configuration file: File is empty or parsing JSON failed";
                if (error != nullptr)
                    qInfo() << "Reported error:" << error->errorString();
            }
            else {
                local = jsonDoc.object().toVariantMap();
                if (local.contains("presentation")) {
                    if (local.contains("notes"))
                        w = new ControlScreen(local.value("presentation").toString(), local.value("notes").toString());
                    else
                        w = new ControlScreen(local.value("presentation").toString());
                    break;
                }
            }
        }
        else {
            // Try to read file as BeamerPresenter drawing binary
            QFile file(parser.positionalArguments()[0]);
            if (file.open(QIODevice::ReadOnly)) {
                QDataStream stream(&file);
                stream.setVersion(QDataStream::Qt_5_0);
                quint32 magic;
                stream >> magic;
                if (stream.status() == QDataStream::Ok && magic == 0x2CA7D9F8) {
                    quint16 version;
                    stream >> version;
                    stream.setVersion(version);
                    QString prespath, notepath;
                    stream >> prespath >> notepath;
                    if (stream.status() != QDataStream::Ok) {
                        qCritical() << "Failed to open drawings file" << parser.positionalArguments()[0] << ". File is corrupt";
                        return 1;
                    }
                    if (!QFileInfo(prespath).exists()) {
                        qCritical() << "Failed to open PDF file" << prespath << "refered to in" << parser.positionalArguments()[0] << ". File does not exist.";
                        return 1;
                    }
                    w = new ControlScreen(prespath, notepath);
                    local["drawings"] = parser.positionalArguments()[0];
                    break;
                }
            }
        }
        }
        qCritical() << "Could not find presentation file. Argument was" << parser.positionalArguments()[0];
        return 1;
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
            if (notesPath == presentationPath)
                w = new ControlScreen(presentationPath);
            else
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

    // Set colors for presentation timer
    if (local.contains("timer")) {
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
    else {
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

    { // Handle key bindings
        QMap<quint32, QStringList> inputMap;
        if (local.contains("keys")) {
            QVariantMap variantMap = local["keys"].value<QVariantMap>();
            quint32 key;
            for (QVariantMap::const_iterator var_it=variantMap.cbegin(); var_it!=variantMap.cend(); var_it++) {
                QKeySequence keySequence = QKeySequence(var_it.key(), QKeySequence::NativeText);
                key = quint32(keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3]);
                if (key == 0)
                    qWarning() << "Could not understand key" << var_it.key();
                else
                    inputMap[key] = var_it->toStringList();
            }
        }
        else {
            settings.beginGroup("keys");
            quint32 key;
            QStringList const keys = settings.allKeys();
            for (QStringList::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                QKeySequence keySequence = QKeySequence(*key_it, QKeySequence::NativeText);
                key = quint32(keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3]);
                if (key == 0)
                    qWarning() << "Could not understand key" << *key_it;
                else
                    inputMap[key] = settings.value(*key_it).toStringList();
            }
            settings.endGroup();
        }
        if (inputMap.isEmpty())
            w->setKeyMap(new QMap<quint32, QList<KeyAction>>(defaultKeyMap));
        else {
            for (QMap<quint32, QStringList>::const_iterator it=inputMap.cbegin(); it!=inputMap.cend(); it++) {
                for (QStringList::const_iterator action_it=it->begin(); action_it!=it->cend(); action_it++) {
                    KeyAction const action = keyActionMap.value(action_it->toLower(), KeyAction::NoAction);
                    if (action != NoAction)
                        w->setKeyMapItem(it.key(), action);
                    else {
                        QStringList split_action = action_it->toLower().split(' ');
                        if (split_action.size() != 2) {
                            qCritical() << "Could not understand action" << *action_it << "for key" << it.key();
                            continue;
                        }
                        quint16 const tool = toolMap.value(split_action.first(), InvalidTool);
                        if (tool == InvalidTool) {
                            qCritical() << "Could not understand action" << *action_it << "for key" << it.key();
                            continue;
                        }
                        QColor color = QColor(split_action[1]);
                        w->setToolForKey(it.key(), {static_cast<DrawTool>(tool), color});
                    }
                }
            }
        }
    }

    { // Configure tool selector (buttons)
        QMap<quint8, QStringList> inputMap;
        if (local.contains("tools")) {
            QVariantMap variantMap = local["tools"].value<QVariantMap>();
            bool ok;
            quint8 key;
            for (QVariantMap::const_iterator var_it=variantMap.cbegin(); var_it!=variantMap.cend(); var_it++) {
                if (var_it.key().length() != 2) {
                    qWarning() << "Could not understand index" << var_it.key() << "which should be a two digit integer.";
                    continue;
                }
                key = quint8(var_it.key().toUShort(&ok, 16));
                if (ok)
                    inputMap[key] = var_it->toStringList();
                else
                    qWarning() << "Could not understand index" << var_it.key() << "which should be a two digit integer.";
            }
        }
        else {
            settings.beginGroup("tools");
            bool ok;
            quint8 key;
            QStringList const keys = settings.allKeys();
            for (QStringList::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                if (key_it->length() != 2) {
                    qWarning() << "Could not understand index" << *key_it << "which should be a two digit integer.";
                    continue;
                }
                key = quint8(key_it->toUShort(&ok, 16));
                if (ok)
                    inputMap[key] = settings.value(*key_it).toStringList();
                else
                    qWarning() << "Could not understand index" << *key_it << "which should be a two digit integer.";
            }
            settings.endGroup();
        }
        if (inputMap.isEmpty())
            w->getToolSelector()->setTools(2, 5, defaultActionMap, defaultColorMap);
        else {
            QMap<quint8, QList<KeyAction>> actionMap;
            QMap<quint8, QColor> colorMap;
            quint8 nrows = 0;
            quint8 ncols = 0;
            for (QMap<quint8, QStringList>::const_iterator it=inputMap.cbegin(); it!=inputMap.cend(); it++) {
                if (16*nrows <= it.key())
                    nrows = quint8(it.key()/16) + 1;
                if (ncols <= it.key()%16)
                    ncols = it.key()%16 + 1;
                actionMap[it.key()] = QList<KeyAction>();
                for (QStringList::const_iterator action_it=it->begin(); action_it!=it->cend(); action_it++) {
                    KeyAction const action = keyActionMap.value(action_it->toLower(), NoAction);
                    if (action != NoAction)
                        actionMap[it.key()].append(static_cast<KeyAction>(action));
                    else {
                        QStringList split_action = action_it->toLower().split(' ');
                        if (split_action.size() != 2) {
                            qWarning() << "Could not understand action" << *action_it;
                            continue;
                        }
                        KeyAction const action = keyActionMap.value(split_action.first(), NoAction);
                        if (action == NoAction)
                            qWarning() << "Could not understand action" << *action_it;
                        else {
                            actionMap[it.key()].append(static_cast<KeyAction>(action));
                            colorMap[it.key()] = QColor(split_action[1]);
                        }
                    }
                }
            }
            w->getToolSelector()->setTools(nrows, ncols, actionMap, colorMap);
        }
    }

    if (local.contains("page times")) { // set times per slide for timer color change
        QMap<int, QTime> map;
        QVariantMap variantMap = local["page times"].value<QVariantMap>();
        bool ok;
        int key;
        QTime time;
        for (QVariantMap::const_iterator it=variantMap.cbegin(); it!=variantMap.cend(); it++) {
            key = it.key().toInt(&ok);
            if (!ok) {
                qCritical() << "In local config / page times: Did not understand slide number" << it.key();
                continue;
            }
            switch(it->toString().count(':')) {
            case 0:
                switch(it->toString().count('.')) {
                case 0:
                    time = QTime::fromString(it->toString(), "m");
                    break;
                case 1:
                    time = QTime::fromString(it->toString(), "m.s");
                    if (!time.isValid()) {
                        time = QTime::fromString(it->toString(), "m.ss");
                        if (!time.isValid()) {
                            time = QTime::fromString(it->toString(), "mm.ss");
                        }
                    }
                    break;
                case 2:
                    time = QTime::fromString(it->toString(), "h.mm.ss");
                    if (!time.isValid())
                        time = QTime::fromString(it->toString(), "h.m.s");
                    break;
                default:
                    qCritical() << "In local config / page times: Did not understand time" << *it;
                    continue;
                }
                break;
            case 1:
                time = QTime::fromString(it->toString(), "m:s");
                if (!time.isValid()) {
                    time = QTime::fromString(it->toString(), "m:ss");
                    if (!time.isValid()) {
                        time = QTime::fromString(it->toString(), "mm:ss");
                    }
                }
                break;
            case 2:
                time = QTime::fromString(it->toString(), "h:mm:ss");
                if (!time.isValid())
                    time = QTime::fromString(it->toString(), "h:m:s");
                break;
            default:
                qCritical() << "In local config / page times: Did not understand time" << *it;
                continue;
            }
            if (time.isValid())
                map.insert(key, time);
            else
                qCritical() << "In local config / page times: Did not understand time" << *it;
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

#ifdef EMBEDDED_APPLICATIONS_ENABLED
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
#endif

    { // Set icon path
        QString icontheme = parser.value("icon-path");
        if (!icontheme.isEmpty() && !QFileInfo(icontheme).isDir()) {
            qWarning() << "icon path" << icontheme << "not understood: should be a directory";
            icontheme = QString();
        }
        if (icontheme.isEmpty())
            icontheme = local.value("icon-theme", "").toString();
        if (!icontheme.isEmpty() && !QFileInfo(icontheme).isDir()) {
            qWarning() << "icon path" << icontheme << "not understood: should be a directory";
            icontheme = QString();
        }
        if (icontheme.isEmpty())
            icontheme = settings.value("icon-theme", "").toString();
        if (!icontheme.isEmpty()) {
            if (QFileInfo(icontheme).isDir()) {
                QIcon::setThemeSearchPaths({icontheme});
                QIcon::setThemeName(icontheme);
            }
            else
                qWarning() << "icon path" << icontheme << "not understood: should be a directory";
        }
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

#ifdef EMBEDDED_APPLICATIONS_ENABLED
        // Set autostart or delay for embedded applications
        value = doubleFromConfig(parser, local, settings, "autostart-emb", -2.);
        emit w->sendAutostartEmbeddedDelay(value);
#endif
    }

    // Settings with integer values
    {
        quint32 value;

        // Set minimum time per frame in animations
        value = intFromConfig<quint32>(parser, local, settings, "min-delay", 40);
        emit w->sendAnimationDelay(value);

        // Set maximum cache size
        value = intFromConfig<quint32>(parser, local, settings, "memory", 100);
        w->setCacheSize(1048576L * value);
    }
    {
        quint8 value;

        // Set number of blinds in blinds slide transition
        value = intFromConfig<quint8>(parser, local, settings, "blinds", 8);
        emit w->setTransitionBlinds(value);

        // Set number of columns in overview
        value = intFromConfig<quint8>(parser, local, settings, "columns", 5);
        if (value<1)
            qCritical() << "You tried to set a number of columns < 1. You shouldn't expect anyting reasonable!";
        else
            w->setOverviewColumns(value);

        // Set number of visible TOC levels
        value = intFromConfig<quint8>(parser, local, settings, "toc-depth", 2);
        if (value<1)
            qCritical() << "You tried to set a number of TOC levels < 1. You shouldn't expect anyting reasonable!";
        else
        w->setTocLevel(value);
    }
    {
        int value;

        // Set scroll step for touch pad input devices
        value = intFromConfig<int>(parser, local, settings, "scrollstep", 200);
        w->setScrollDelta(value);

        // Set maximum number of cached slides
        value = intFromConfig<int>(parser, local, settings, "cache", -1);
        w->setCacheNumber(value);
    }
    {
        quint16 value;

        // Set size of draw tools
        value  = intFromConfig<quint16>(parser, local, settings, "magnifier-size", 120);
        w->setToolSize(Magnifier, value);
        value  = intFromConfig<quint16>(parser, local, settings, "pointer-size", 10);
        w->setToolSize(Pointer, value);
        value  = intFromConfig<quint16>(parser, local, settings, "torch-size", 80);
        w->setToolSize(Torch, value);
        value  = intFromConfig<quint16>(parser, local, settings, "highlighter-width", 30);
        w->setToolSize(Highlighter, value);
        value  = intFromConfig<quint16>(parser, local, settings, "pen-width", 3);
        w->setToolSize(Pen, value);
        value  = intFromConfig<quint16>(parser, local, settings, "eraser-size", 10);
        w->setToolSize(Eraser, value);
    }

    // Disable slide transitions
    if (parser.isSet("d"))
        w->disableSlideTransitions();
    else if (local.contains("no-transitions")) {
        // This is rather unintuitive. Just set any value...
        QString string = local.value("no-transitions").toString().toLower();
        if (QStringList({"", "true", "no-transitions", "no transitions", "1"}).contains(string))
            w->disableSlideTransitions();
    }
    else if (settings.contains("no-transitions"))
        w->disableSlideTransitions();

    // Treat all scroll inputs as touch pads
    if (parser.isSet("force-touchpad"))
        w->setForceTouchpad();
    else if (local.contains("force-touchpad")) {
        // This is rather unintuitive. Just set any value...
        QString string = local.value("force-touchpad").toString().toLower();
        if (QStringList({"", "true", "force touchpad", "1"}).contains(string))
            w->setForceTouchpad();
    }
    else if (settings.contains("force-touchpad"))
        w->setForceTouchpad();

    // Settings, which can cause exceptions

    // Set presentation time
    try {
        if (parser.value("t").isEmpty())
            throw 0;
        emit w->sendTimerString(parser.value("t"));
    }
    catch (int const) {
        try {
            if (!local.contains("time"))
                throw 0;
            emit w->sendTimerString(local.value("time").toStringList()[0]);
        }
        catch (int const) {
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
                catch (int const) {
                    qCritical() << "Failed to set custom renderer from configuration";
                }
            }
            if (err==-1)
                qCritical() << "Ignored request to use undefined custom renderer";
        }
    }


    { // Show the GUI depending on the QPA backend
        bool showNotes = !parser.isSet("n");
        if (showNotes && local.contains("no-notes")) {
            // This is rather unintuitive. Just set any value...
            QString string = local.value("no-notes").toString().toLower();
            if (QStringList({"", "true", "no-notes", "1"}).contains(string))
                showNotes = false;
        }
        else if (settings.contains("no-notes") && !parser.isSet("force-show"))
            showNotes = false;
        // show the GUI
        if (QStringList({"xcb", "wayland", "wayland-egl"}).contains(app.platformName())) {
            if (showNotes) {
                w->show();
                w->activateWindow();
            }
        }
        else if (QStringList({"wayland-xcomposite-egl", "wayland-xcomposite-glx", "windows", "direct2d","iOS", "cocoa", "haiku"}).contains(app.platformName())) {
            qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback would be very welcome!";
            qInfo() << "Using QPA platform plugin" << app.platformName();
            if (showNotes) {
                w->show();
                w->activateWindow();
            }
        }
        else if (QStringList({"qnx", "android", "minimal", "minimalegl", "offscreen", "vnc", "mirclient", "winrt"}).contains(app.platformName())) {
            if (parser.isSet("force-show")) {
                qWarning() << "You are using a QPA platform plugin, which might cause problems when trying to show two windows.";
                qInfo() << "QPA platform plugin is:" << app.platformName();
                qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback would be very welcome!";
                w->show();
                w->activateWindow();
            }
            else {
                qWarning() << "You are using a QPA platform plugin, which might cause problems when trying to show two windows. Showing only the presentation window.";
                qInfo() << "You can force showing notes with the option --force-show";
                qInfo() << "QPA platform plugin is:" << app.platformName();
                qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback would be very welcome!";
            }
        }
        else if (QStringList({"eglfs", "linuxfb", "directfb", "kms", "openwfd", "bsdfb"}).contains(app.platformName())) {
            qCritical() << "You are using a QPA platform plugin, on which BeamerPresenter might freeze and block your system.";
            qInfo() << "QPA platform plugin is:" << app.platformName();
            if (parser.isSet("force-show")) {
                qWarning() << "Continuing anyway, because you have set --force-show";
                qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback would be very welcome!";
            }
            else {
                qInfo() << "Exiting now to avoid crashing you system. You can force showing the presentation with --force-show.";
                return 1;
           }
        }
        else {
            qWarning() << "You are using an unknown QPA platform plugin.";
            qInfo() << "QPA platform plugin is:" << app.platformName();
            qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback would be very welcome!";
            if (showNotes) {
                qInfo() << "If your platform (plugin) is not able to show two windows, please use the option -n or --no-notes.";
                w->show();
                w->activateWindow();
            }
        }
    }

    // Render first page on presentation screen
    emit w->sendNewPageNumber(0, false);
    // Render first page on control screen
    w->renderPage(0);

    if (local.contains("drawings")) {
        QString drawpath = local.value("drawings").toString();
        w->loadDrawings(drawpath);
    }

    // Here one could update the cache.
    // But you probably first want to adjust the window size and then update it with key shortcut c.
    //w->updateCache();

    // start the execution loop
    int status = app.exec();
    delete w;
    return status;
}
