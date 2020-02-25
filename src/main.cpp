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
#include "screens/controlscreen.h"
#include "names.h"


/// Read real value from string (handling % sign correctly).
/// Throw useful exceptions if string represents a boolean value.
qreal stringToReal(QString& string)
{
    bool ok;
    qreal result = string.toDouble(&ok);
    if (!ok && string.endsWith("%")) {
        string.chop(1);
        result = 0.01*string.toDouble(&ok);
    }
    if (ok) {
        if (result >= 0.)
            return result;
        else
            throw 2;
    }
    if (string == "true")
        throw 0;
    if (string == "false")
        throw 1;
    throw -1;
}


/// Read options from command line arguments, local configuration and global configuration file which should have a nonnegative floating point or boolean value.
/// If the argument is "true", return 0; If the argument is "false" return -2. If value is set for option <name>, return <return_value>.
qreal qrealFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, qreal const return_value, qreal const max=1e30)
{
    qreal result;
    QString value;
    // Check whether the option is set in the command line arguments.
    if (parser.isSet(name)) {
        value = parser.value(name).toLower();
        try {
            result = stringToReal(value);
            if (result < max)
                return result;
        }
        catch(int error) {
            switch(error) {
            case 0:
                return 0.;
            case 1:
                return -2.;
            }
        }
        qWarning() << "option" << value << "to" << name << "not understood. Should be 0 <= number <" << max << "or true/false.";
    }
    // Check whether a local configuration file contains the option.
    if (local.contains(name)) {
        value = local.value(name).toString().toLower();
        try {
            result = stringToReal(value);
            if (result < max)
                return result;
        }
        catch(int error) {
            switch(error) {
            case 0:
                return 0.;
            case 1:
                return -2.;
            }
        }
        qWarning() << "option" << value << "to" << name << "in local config not understood. Should be 0 <= number <" << max << "or true/false.";
    }
    // Check whether the global configuration file contains the option.
    if (settings.contains(name)) {
        value = settings.value(name).toString().toLower();
        try {
            result = stringToReal(value);
            if (result < max)
                return result;
        }
        catch(int error) {
            switch(error) {
            case 0:
                return 0.;
            case 1:
                return -2.;
            }
        }
        qWarning() << "option" << value << "to" << name << "in config not understood. Should be 0 <= number <" << max << "or true/false.";
    }
    // Return default value.
    return return_value;
}


/// Read options from command line arguments, local configuration and global configuration file which should have a boolean value.
/// If option <name> is not set or the given arguments are invalid, return <return_value>.
bool boolFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, bool const return_value)
{
    QString value;
    // Check whether the option is set in the command line arguments.
    if (parser.isSet(name)) {
        value = parser.value(name).toLower();
        // Try to interpret the value as true or false.
        if (value == "true")
            return true;
        else if (value == "false")
            return false;
        else
            qWarning() << "option" << parser.value(name) << "to" << name << "not understood. Should be true or false.";
    }
    // Check whether a local configuration file contains the option.
    if (local.contains(name)) {
        value = local.value(name).toString().toLower();
        // Try to interpret the value as true or false.
        if (value == "true")
            return true;
        else if (value == "false")
            return false;
        else
            qWarning() << "option" << local.value(name) << "to" << name << "in local config not understood. Should be true or false.";
    }
    // Check whether the global configuration file contains the option.
    if (settings.contains(name)) {
        value = settings.value(name).toString().toLower();
        // Try to interpret the value as true or false.
        if (value == "true")
            return true;
        else if (value == "false")
            return false;
        else
            qWarning() << "option" << settings.value(name) << "to" << name << "in config not understood. Should be true or false.";
    }
    // Return default value.
    return return_value;
}


/// Read options from command line arguments, local configuration and global configuration file which should have an integer (type <T>) value.
/// If option <name> is not set or the given arguments are invalid, return <return_value>.
template <class T>
T intFromConfig(QCommandLineParser const& parser, QVariantMap const& local, QSettings const& settings, QString name, T const return_value)
{
    bool ok;
    // Check whether type T is signed.
    bool const is_signed = T(-1) < 0;
    T result;
    // Check whether the option is set in the command line arguments.
    if (parser.isSet(name)) {
        // Try to interpret the value as an integer.
        if (is_signed)
            result = T(parser.value(name).toInt(&ok));
        else
            result = T(parser.value(name).toUInt(&ok));
        if (ok)
            return result;
        else
            qWarning() << "option" << parser.value(name) << "to" << name << "not understood. Should be an integer.";
    }
    // Check whether a local configuration file contains the option.
    if (local.contains(name)) {
        // Try to interpret the value as an integer.
        if (is_signed)
            result = T(local.value(name).toInt(&ok));
        else
            result = T(local.value(name).toUInt(&ok));
        if (ok)
            return result;
        else
            qWarning() << "option" << local.value(name) << "to" << name << "in local config not understood. Should be an integer.";
    }
    // Check whether the global configuration file contains the option.
    if (settings.contains(name)) {
        // Try to interpret the value as an integer.
        if (is_signed)
            result = T(settings.value(name).toInt(&ok));
        else
            result = T(settings.value(name).toUInt(&ok));
        if (ok)
            return result;
        else
            qWarning() << "option" << settings.value(name) << "to" << name << "in config not understood. Should be an integer.";
    }
    // Return default value.
    return return_value;
}


/// Collect and interpret group of arguments representing key actions or draw tools.
void actionsFromConfig(QMap<QString, QList<KeyAction>>& actions, QMap<QString, FullDrawTool>& tools, QVariantMap const& local, QSettings& settings, QString name)
{
    // Read the values from local or global config to a common format.
    QMap<QString, QStringList> basic;
    QMap<QString, QMap<QString, QVariant>> advanced;
    // Try to read input from local config.
    if (local.contains(name)) {
        QMap<QString, QVariant> map = local[name].toMap();
        for (QMap<QString, QVariant>::const_iterator item = map.cbegin(); item != map.cend(); item++) {
            if (item->canConvert<QVariantMap>())
                advanced[item.key()] = item->toMap();
            else if (item->canConvert<QStringList>())
                basic[item.key()] = item->toStringList();
            else if (item->canConvert<QString>())
                basic[item.key()] = item->toString().split(",");
            else
                qWarning() << "Could not understand data type:" << *item;
        }
    }
    if (basic.isEmpty() && advanced.isEmpty()) {
        // Try to read input from global config.
        settings.beginGroup(name);
        for (auto key : settings.childGroups()) {
            settings.beginGroup(key);
            advanced[key] = QMap<QString, QVariant>();
            for (auto subkey : settings.allKeys())
                advanced[key][subkey] = settings.value(subkey);
            settings.endGroup();
        }
        for (auto key : settings.childKeys()) {
            QVariant const variant = settings.value(key);
            if (variant.canConvert<QStringList>())
                basic[key] = variant.toStringList();
            else if (variant.canConvert<QString>())
                basic[key] = variant.toString().split(",");
            else
                qWarning() << "Could not understand data type:" << variant;
        }
        settings.endGroup();
    }

    // Interpret the values as key actions and tools.
    for (QMap<QString, QStringList>::const_iterator it = basic.cbegin(); it != basic.cend(); it++) {
        // Iterate over all actions (refered to as "action string" in the following) defined for each key (usually this is only one).
        for (QStringList::const_iterator action_it=it->begin(); action_it!=it->cend(); action_it++) {
            KeyAction const action = keyActionMap.value(action_it->toLower(), KeyAction::NoAction);
            // Remove spaces from key:
            QString key = it.key();
            key.remove(' ');
            // If a KeyAction was found in keyActionMap for this action string: send this key binding to ctrlScreen.
            if (action != NoAction)
                actions[key].append(action);
            else {
                try {
                    // Try to interpret  as a sequence "tool color" or "tool color size", defining a drawing or highlighting tool and a color.
                    QStringList const split_action = action_it->toLower().split(' ');
                    // Convert the first word of the action string as a draw tool.
                    DrawTool const tool = toolMap.value(split_action.first(), InvalidTool);

                    // Actually tool != InvalidTool should imply split_action.size() > 1.
                    // But just to be sure, we can check again.
                    if (tool == InvalidTool || split_action.size() < 2)
                        throw 1;

                    if (action_it->contains('=')) {
                        // Interpret all arguments as key=value pairs.
                        QMap<QString, QString> map;
                        for (QStringList::const_iterator attr = split_action.cbegin()+1; attr != split_action.cend(); attr++) {
                            QStringList const keyval = attr->split('=');
                            if (keyval.size() != 2)
                                throw 2;
                            map[keyval.first()] = keyval.last();
                        }
                        QColor const color = QColor(map.value("color"));
                        qreal const size = map.value("size").toDouble();
                        if (tool == Magnifier) {
                            bool ok;
                            qreal const magnification = map.value("magnification").toDouble(&ok);
                            if (ok)
                                tools[key] = {tool, color, size, {magnification}};
                            else
                                tools[key] = {tool, color, size};
                        }
                        else
                            tools[key] = {tool, color, size};
                    }
                    else {
                        // Interpret all following arguments as color, size and magnification.
                        QColor color = QColor(split_action[1]);
                        qreal size = 0;
                        bool ok = false;
                        if (!color.isValid()) {
                            size = split_action[1].toDouble(&ok);
                            if (ok && split_action.size() == 3)
                                color = QColor(split_action[2]);
                        }
                        if (!ok && split_action.size() == 3)
                            size = split_action[2].toDouble(&ok);
                        if (tool == Magnifier) {
                            qreal magnification = 0.;
                            if (split_action.size() > 3)
                                magnification = split_action[3].toDouble();
                            tools[key] = {tool, color, size, {magnification}};
                            if (split_action.size() > 4)
                                qWarning() << "Tool has too many arguments:" << it.key() << *it;
                        }
                        else {
                            tools[key] = {tool, color, size};
                            if (split_action.size() > 3)
                                qWarning() << "Tool has too many arguments:" << it.key() << *it;
                        }
                    }
                } catch (int) {
                    qCritical() << "Could not understand action" << *action_it << "for key" << it.key();
                }
            }
        }
    }
    for (QMap<QString, QMap<QString, QVariant>>::const_iterator it = advanced.cbegin(); it != advanced.cend(); it++) {
        try {
            if (!it->contains("tool"))
                throw 0;
            // Convert the first word of the action string as a draw tool.
            quint16 const tool = toolMap.value(it->value("tool").toString(), InvalidTool);
            if (tool == InvalidTool)
                throw 1;
            // Parse the second word of the action string as a QColor.
            QColor const color = QColor(it->value("color").toString());
            qreal const size = it->value("size").toDouble();
            // Remove spaces from key:
            QString key = it.key();
            key.remove(' ');
            if (tool == Magnifier) {
                qreal const magnification = it->value("magnification").toDouble();
                tools[key] = {static_cast<DrawTool>(tool), color, size, {magnification}};
            }
            else
                tools[key] = {static_cast<DrawTool>(tool), color, size};
        } catch (int) {
            qCritical() << "Could not understand action" << *it << "for key" << it.key();
        }
    }
}


/// Main function
int main(int argc, char *argv[])
{
    // Set format for debugging output, warnings etc.
    // To overwrite this you can set the environment variable QT_MESSAGE_PATTERN.
    qSetMessagePattern("%{time process} %{if-debug}D%{endif}%{if-info}INFO%{endif}%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}%{if-category} %{category}%{endif}%{if-debug} %{file}:%{line}%{endif} - %{message}%{if-fatal} from %{backtrace [depth=3]}%{endif}");

    // Set up the application.
    QApplication app(argc, argv);
    app.setApplicationName("BeamerPresenter");
    // Set app version. The string APP_VERSION is defined in beamerpresenter.pro.
#ifdef QT_DEBUG
#ifdef POPPLER_VERSION
    app.setApplicationVersion(APP_VERSION " debugging (poppler=" POPPLER_VERSION ", Qt=" QT_VERSION_STR ")");
#else
    app.setApplicationVersion(APP_VERSION " debugging (Qt=" QT_VERSION_STR ")");
#endif
#else
#ifdef POPPLER_VERSION
    app.setApplicationVersion(APP_VERSION " (poppler=" POPPLER_VERSION ", Qt=" QT_VERSION_STR ")");
#else
    app.setApplicationVersion(APP_VERSION " (Qt=" QT_VERSION_STR ")");
#endif
#endif

    // Set up command line argument parser.
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
    // Define command line options.
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    parser.addPositionalArgument("<notes.pdf>",  "Notes for the presentation (optional, should have the same number of pages as <slides.pdf>)");
    // TODO: change letters for option shortcuts
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
        {{"g", "glitter-pixel"}, "size of glitter pixel in glitter slide transition", "int"},
        {{"G", "glitter-steps"}, "number of independent random glitter pixels in glitter slide transition", "int"},
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
        {{"V", "video-cache"}, "Preload videos for the following slide.", "bool"},
#ifdef EMBEDDED_APPLICATIONS_ENABLED
        {{"w", "pid2wid"}, "Program that converts a PID to a Window ID.", "file"},
        {{"x", "log"}, "Log times of slide changes to standard output."},
#endif
        {"color-frames", "Minimum number of frames used for each color transitions in timer colors.", "int"},
#ifdef CHECK_QPA_PLATFORM
        {"force-show", "Force showing notes or presentation (if in a framebuffer) independent of QPA platform plugin."},
#endif
        {"force-touchpad", "Treat every scroll input as touch pad."},
        {"sidebar-width", "Minimum relative width of sidebar on control screen. Number between 0 and 1.", "float"},
        {"mute-presentation", "Mute presentation (default: false)", "bool"},
        {"mute-notes", "Mute notes (default: true)", "bool"},
        {"eraser-size", "Radius of eraser.", "pixels"},
        {"icon-path", "Set path for default icons, e.g. /usr/share/icons/default", "path"},
    });
    parser.process(app);

    // Set up a settings manager
    // This is basically designed for GNU/Linux, where it loads $dir/beamerpresenter/beamerpresenter.conf for dir in $XDG_CONFIG_DIRS.
    // On MS Windows this (probably) tries to load a file USER_HOME\AppData\Roaming\beamerpresenter.ini or USER_HOME\AppData\Roaming\beamerpresenter\beamerpresenter.ini.
    // Please use the Qt documentation for more details.
#ifdef Q_OS_LINUX
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#else
#ifdef Q_OS_WIN
    qInfo() << "BeamerPresenter is not regularly tested on MS Windows. Please send detailed bug reports if you encounter problems while using BeamerPresenter.";
    qInfo() << "Bugs can be reported on https://github.com/stiglers-eponym/BeamerPresenter/issues";
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#else
#ifdef Q_OS_MACOS
    qInfo() << "BeamerPresenter has not been tested on MacOS. Please send detailed bug reports if you encounter problems while using BeamerPresenter.";
    qInfo() << "Bugs can be reported on https://github.com/stiglers-eponym/BeamerPresenter/issues";
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter"); // untested!
#else
    qInfo() << "BeamerPresenter has not been tested on your OS.";
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter", "beamerpresenter");
#endif
#endif
#endif
    qDebug() << "Loaded settings:" << settings.allKeys(); // Show all settings in plain text.

    // Load an optional local configuration file.
    // This file can be used to set e.g. times per slide.

    /// QVariantMap containing all local configuration options.
    /// This stays empty if no local configuration file is given.
    QVariantMap local = QVariantMap();
    // If a local configuration is given using the command line option "-j" or "--json":
    if (!parser.value("j").isEmpty()) {
        // Try to open the local configuration file.
        QFile jsonFile(parser.value("j"));
        if (jsonFile.exists()) {
            jsonFile.open(QFile::ReadOnly);
            QJsonParseError* error = nullptr;
            // Parse the file as a JSON file.
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), error);
            // Check for errors.
            if (jsonDoc.isNull() || jsonDoc.isEmpty()) {
                qCritical() << "Failed to load local configuration file: File is empty or parsing JSON failed";
                if (error != nullptr)
                    qInfo() << "Reported error:" << error->errorString();
            }
            else {
                // Write the options in local.
                local = jsonDoc.object().toVariantMap();
            }
        }
        else
            qCritical() << "Failed to load local configuration file: File doesn't exist.";
    }


    // Handle positional arguments.
    // These should be either: 1 pdf file (the presentation) or 2 pdf files (presentation and notes) or 1 json file (the local configuration) or 1 BeamerPresenter drawings file.
    QString presentation, notes;
    switch (parser.positionalArguments().size()) {
    case 1:
        {
        // Check whether the argument is a file.
        if (!QFileInfo(parser.positionalArguments()[0]).exists()) {
            qCritical() << "File" << parser.positionalArguments()[0] << "does not exist!";
            return 1;
        }
        // Get the file type using mime types.
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForFile(parser.positionalArguments()[0], QMimeDatabase::MatchContent);
        if (!type.isValid()) {
            type = db.mimeTypeForFile(parser.positionalArguments()[0], QMimeDatabase::MatchExtension);
            if (!type.isValid()) {
                qCritical() << "Did not understand type of file" << parser.positionalArguments()[0];
                return 1;
            }
        }
        // If file is a pdf file, it is interpreted as the presentation file.
        if (type.suffixes().contains("pdf", Qt::CaseInsensitive)) {
            // Set up ctrlScreen using the argument as the presentation file.
            presentation = parser.positionalArguments()[0];
            break;
        }
        // If file is a JSON file, it is interpreted as a local configuration file.
        if (type.suffixes().contains("txt") || type.suffixes().contains("json")) {
            // Try to open the file.
            QFile jsonFile(parser.positionalArguments()[0]);
            jsonFile.open(QFile::ReadOnly);
            QJsonParseError* error = nullptr;
            // Parse the file as JSON document.
            QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonFile.readAll(), error);
            // Check for errors.
            if (jsonDoc.isNull() || jsonDoc.isEmpty()) {
                qWarning() << "Failed to load local configuration file: File is empty or parsing JSON failed";
                if (error != nullptr)
                    qInfo() << "Reported error:" << error->errorString();
            }
            else {
                // Write the options from the JSON file in local.
                local = jsonDoc.object().toVariantMap();
                // Check whether the JSON document contains the keys "presentation" and "notes".
                // Use these corresponding options as files to construct ctrlScreen.
                if (local.contains("presentation")) {
                    presentation = local.value("presentation").toString();
                    if (local.contains("notes"))
                        notes = local.value("notes").toString();
                    break;
                }
                else {
                    qCritical() << "Could not find presentation file. Argument was" << parser.positionalArguments()[0];
                    return 1;
                }
            }
        }

        // Try to read file as BeamerPresenter drawing file (XML, compressed XML or legacy binary format).
        QFile file(parser.positionalArguments()[0]);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Cannot read file:" << parser.positionalArguments()[0];
            return 1;
        }
        // Try to open the file as (compressed) XML document.
        QDomDocument doc("BeamerPresenter");
        if (!doc.setContent(&file)) {
            file.close();
            file.open(QIODevice::ReadOnly);
            if (!doc.setContent(qUncompress(file.readAll()))) {
                file.close();
                doc.clear();
            }
        }
        if (doc.isNull()) {
            // Deprecated
            // Try to read deprecated binary file format.
            file.open(QIODevice::ReadOnly);
            // Read the file in a QDataStream.
            QDataStream stream(&file);
            stream.setVersion(QDataStream::Qt_5_0);
            // Read "magic bytes" from the stream an compare.
            quint32 magic;
            stream >> magic;
            // Check for errors and whether the magic bytes match the ones defined for BeamerPresenter binaries.
            if (stream.status() == QDataStream::Ok && magic == 0x2CA7D9F8) {
                // Read QDataStream version from stream.
                quint16 version;
                stream >> version;
                // Overwrite QDataStream version with the version read from the stream.
                stream.setVersion(version);
                // Read file paths for presentation and notes.
                stream >> presentation >> notes;
                // Check for errors.
                if (stream.status() != QDataStream::Ok) {
                    qCritical() << "Failed to open drawings file" << parser.positionalArguments()[0] << ". File is corrupt";
                    return 1;
                }
            }
        }
        else {
            QDomElement const root = doc.documentElement();
            if (root.attribute("creator").contains("beamerpresenter", Qt::CaseInsensitive)) {
                QDomElement const pres_element = root.firstChildElement("presentation");
                presentation = pres_element.attribute("file");
                QDomElement const notes_element = root.firstChildElement("notes");
                notes = notes_element.attribute("file");
            }
            else if (root.attribute("creator").contains("xournal", Qt::CaseInsensitive)) {
                // Try to get a filename.
                QDomElement const first_page = root.firstChildElement("page");
                QDomElement const first_bg = first_page.firstChildElement("background");
                presentation = first_bg.attribute("filename");
            }
            else {
                qCritical() << "Failed to understand file: Unknown creator" << root.attribute("creator");
                return 1;
            }
        }
        // Check whether presentation file exists.
        // This check could be left out (it is repeated in the constructure of ControlScreen), but like this it provides a more detailed error message.
        if (!QFileInfo(presentation).exists()) {
            qCritical() << "Failed to open PDF file" << presentation << "refered to in" << parser.positionalArguments()[0] << ". File does not exist.";
            return 1;
        }
        // Create ctrlScreen.
        // notespath can be empty, which is handled correctly in the constructure of ControlScreen.
        // Set the drawings file as a drawings file in local configuration.
        // Later this option in local will be used to load the drawings saved in the drawings file.
        local["drawings"] = parser.positionalArguments()[0];
        break;
        }
    case 2:
        // Two positional arguments are given.
        // Assume that these are the two PDF files for presentation and notes.
        // All checks are done in the constructur of ControlScreen.
        // TODO: allow first file to be an uncompressed Xournal(++) file.
        presentation = parser.positionalArguments()[0];
        notes = parser.positionalArguments()[1];
        break;
    case 0:
        // No positional arguments given.
        // Check whether a local configuration file defines a local presentation file.
        if (local.contains("presentation")) {
            // Check whether notes are also given and create ctrlScreen.
            presentation = local.value("presentation").toString();
            if (local.contains("notes"))
                notes = local.value("notes").toString();
        }
        else {
            // Find pdf files using a QFileDialog.
            // First open a QFileDialog to find a presentation file.
            /// QString containing path of presentation file
            presentation = QFileDialog::getOpenFileName(nullptr, "Open Slides", "", "Documents (*.pdf)");
            // Check whether a file was selected.
            if (presentation.isEmpty()) {
                qCritical() << "No presentation file specified";
                // Exit with error showing help message.
                parser.showHelp(1);
            }
            // Check if the presentation file should include notes (beamer option show notes on second screen).
            // If the presenation file is interpreted as a normal presentation, open a note file in a QFileDialog.
            if (parser.value("p").isEmpty()) {
                if (!settings.contains("page-part") || settings.value("page-part").toString()=="none" || settings.value("page-part").toString()=="0") {
                    // Option "page-part" (indicating that the presentation should contain notes) is not set.
                    // Open notes in second QFileDialog. By default the presentation file is selected.
                    notes = QFileDialog::getOpenFileName(nullptr, "Open Notes", presentation, "Documents (*.pdf)");
                }
            }
            else if (parser.value("p")=="none" || parser.value("p")=="0") {
                // The value of "page-part" explicitly states that the presentation file does not contain notes.
                // Open notes in second QFileDialog. By default the presentation file is selected.
                notes = QFileDialog::getOpenFileName(nullptr, "Open Notes", presentation, "Documents (*.pdf)");
            }
            // Create ctrlScreen.
        }
        break;
    default:
        // Incompatible number of arguments given.
        qCritical() << "Received more than 2 positional arguments.";
        // Exit with error showing help message.
        parser.showHelp(1);
    }


    /// ControlScreen widget managing everything.
    ControlScreen* ctrlScreen;
    // Create ctrlScreen and thereby the whole GUI.
    {
        PagePart pagePart = FullPage;
        // Split page if necessary (beamer option show notes on second screen).
        // With this option half of the pages in a pdf document contain the presentation and the other half contains the notes.
        bool found = false;
        // Check whether the command line arguments contain the option "page-part"
        if (!parser.value("p").isEmpty()) {
            QString value = parser.value("p");
            if ( value == "r" || value == "right" ) {
                pagePart = RightHalf;
                found = true;
            }
            else if ( value == "l" || value == "left" ) {
                pagePart = LeftHalf;
                found = true;
            }
            else if (value != "none" && value != "0")
                qCritical() << "option \"" << value << "\" to page-part not understood.";
        }
        // Check whether the local configuration contains the option "page-part"
        if (!found && local.contains("page-part")) {
            QString value = settings.value("page-part").toString();
            if ( value == "r" || value == "right" ) {
                pagePart = RightHalf;
                found = true;
            }
            else if ( value == "l" || value == "left" ) {
                pagePart = LeftHalf;
                found = true;
            }
            else if (value != "none" && value != "0")
                qCritical() << "option \"" << value << "\" to page-part in local config not understood.";
        }
        // Check whether the global configuration contains the option "page-part"
        if (!found && settings.contains("page-part")) {
                QString value = settings.value("page-part").toString();
                if ( value == "r" || value == "right" )
                    pagePart = RightHalf;
                else if ( value == "l" || value == "left" )
                    pagePart = LeftHalf;
                else if (value != "none" && value != "0")
                    qCritical() << "option \"" << value << "\" to page-part in config not understood.";
        }

        // Create the GUI.
        /// ctrlScreen will be the object which manages everything.
        /// It is the window shown on the speaker's monitor.
        /// If the option "-n" or "--no-notes" is set, ctrlScreen is not shown, but still controls everything.
        ctrlScreen = new ControlScreen(presentation, notes, pagePart);
        // In the following ctrlScreen will be created using the positional arguments.
        // When ctrlScreen is created, the presentation is immediately shown.
    }


    // Handle options from command line and configuration files.

    // Set colors of presentation and control screen as defined in local or global configuration file.
    {   // Set background colors for presentation and control screen, and text color for control screen.
        QColor bgColor, textColor, presentationColor;

        // notes background color (bgColor): background of the control screen
        // Try to read bgColor from local configuration file.
        if (local.contains("notes background color"))
            bgColor = local.value("notes background color").value<QColor>();
        // Try to read bgColor from global configuration file.
        else if (settings.contains("notes background color"))
            bgColor = settings.value("notes background color").value<QColor>();
        // Set default value for bgColor.
        else
            bgColor = Qt::gray;

        // notes text color (textColor): text color on the control screen
        if (local.contains("notes text color"))
            textColor = local.value("notes text color").value<QColor>();
        else if (settings.contains("notes text color"))
            textColor = settings.value("notes text color").value<QColor>();
        else
            textColor = Qt::black;

        // presentation color (presentationColor): background color on the presentation screen
        if (local.contains("presentation color"))
            presentationColor = local.value("presentation color").value<QColor>();
        else if (settings.contains("presentation color"))
            presentationColor = settings.value("presentation color").value<QColor>();
        else
            presentationColor = Qt::black;

        // Parse the colors of the control screen to ctrlScreen.
        ctrlScreen->setColor(bgColor, textColor);
        // Parse the presentation color to the ctrlScreen (which controls also the presentation screen).
        ctrlScreen->setPresentationColor(presentationColor);
    }


    // Handle arguments which define arrays or maps of variable size.

    // Set colors for presentation timer.
    // This can be used to define custom color changes depending on the time left for the presentation or for a specific slide.
    {
        /// Time arguments
        QStringList keys;
        /// Color arguments if colors are given in local configuration file; empty otherwise
        QVariantMap variantMap;

        // Check whether the argument is set in the local configuration file.
        if (local.contains("timer")) {
            /// Subset of arguments from local configuration which define the timer colors.
            /// All these arguments should have the form (key, value) = (int, color).
            variantMap = local["timer"].value<QVariantMap>();
            keys = variantMap.keys();
        }
        else {
            // If the local configuration file does not contain an argument for the timer colors,
            // try to read the colors from the global configuration file.
            // The timer colors form a group of arguments in the global configuration.
            // Enter this "timer" group.
            settings.beginGroup("timer");
            keys = settings.childKeys();
        }

        // If no colors are set, send the default configuration.
        if (keys.isEmpty())
            ctrlScreen->getTimer()->setColors({-150000, 0, 150000}, {Qt::white, Qt::green, Qt::red});
        else {
            // Convert the arguments from variantMap or settings to integers and colors.
            // Use a QMap to store the values, because it is automatically sorted by the keys.
            /// Translation of variantMap or settings with types casted to int and QColor.
            QMap<qint32, QColor> map;
            bool ok;
            qint32 time;
            QColor color;
            // Iterate over variantMap to copy and convert it to map.
            for (QStringList::const_iterator key_it=keys.cbegin(); key_it!=keys.cend(); key_it++) {
                // Parse the keys as an integer.
                time = 1000*(*key_it).toDouble(&ok);
                if (ok) {
                    // Parse the value argument as a QColor.
                    // Check whether the value from settings or variantMap should be used.
                    if (variantMap.isEmpty())
                        color = settings.value(*key_it).value<QColor>();
                    else
                        color = variantMap[*key_it].value<QColor>();
                    if (color.isValid()) {
                        // If successful: Write key and value to map.
                        map[time] = color;
                    }
                    else {
                        // (key,value) pairs which cause errors will be ignored and cause an error message.
                        if (variantMap.isEmpty())
                            qCritical() << "Color" << settings.value(*key_it) << "not understood. Should be a color.";
                        else
                            qCritical() << "Color" << variantMap[*key_it].toString() << "not understood. Should be a color.";
                    }
                }
                else {
                    // (key,value) pairs which cause errors will be ignored and cause an error message.
                    qCritical() << "Time interval" << *key_it << "not understood. Should be an integer.";
                }
            }
            // Send the times and colors to timer GUI.
            // The list map.keys() is sorted. map.values() has the same order as map.keys().
            // The arguments in the configuration file do not need to be sorted.
            ctrlScreen->getTimer()->setColors(map.keys(), map.values());

            // Get minimum number of frames for color transitions from config.
            quint16 const min_frames = intFromConfig<quint16>(parser, local, settings, "color-frames", 25);
            ctrlScreen->getTimer()->updateGuiInterval(min_frames);
        }
        // Check whether we used settings and opened a group there.
        if (variantMap.isEmpty())
            // Exit the "timer" group.
            settings.endGroup();
    }


    // Handle key bindings.
    {   // In the configuration actions can be defined for different keyboard actions.
        // The arguments are first read from the local or global configuration to the map "inputMap" before the arguments are interpreted.

        // First set static key actions.
        // This sends the real key for some navigation keys used in modes like overview and TOC mode.
        for (QMap<quint32, KeyAction>::const_iterator it=staticKeyMap.cbegin(); it!=staticKeyMap.cend(); it++)
            ctrlScreen->setKeyMapItem(it.key(), *it);

        QMap<QString, QList<KeyAction>> actions;
        QMap<QString, FullDrawTool> tools;
        actionsFromConfig(actions, tools, local, settings, "keys");

        // If no key bindings have been defined: Send the default key map to ctrlScreen.
        if (actions.isEmpty()) {
            ctrlScreen->setKeyMap(new QMap<quint32, QList<KeyAction>>(defaultKeyMap));
        }
        // Send key actions to control screen.
        for (QMap<QString, QList<KeyAction>>::const_iterator it = actions.cbegin(); it != actions.cend(); it++) {
            // Parse the key as a QKeySequence.
            QKeySequence const keySequence = QKeySequence(it.key(), QKeySequence::NativeText);
            // Convert the key sequence to an integer.
            quint32 const key = quint32(keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3]);
            if (key == 0)
                qWarning() << "Could not understand key" << it.key();
            else {
                // Iterate over all actions (refered to as "action string" in the following) defined for each key (usually this is only one).
                for (QList<KeyAction>::const_iterator action=it->cbegin(); action!=it->cend(); action++)
                    ctrlScreen->setKeyMapItem(key, *action);
            }
        }
        // Send tools to control screen.
        for (QMap<QString, FullDrawTool>::const_iterator it = tools.cbegin(); it != tools.cend(); it++) {
            // Parse the key as a QKeySequence.
            QKeySequence const keySequence = QKeySequence(it.key(), QKeySequence::NativeText);
            // Convert the key sequence to an integer.
            quint32 const key = quint32(keySequence[0]+keySequence[1]+keySequence[2]+keySequence[3]);
            if (key == 0)
                qWarning() << "Could not understand key" << it.key();
            else
                ctrlScreen->setToolForKey(key, *it);
        }
    }


    // Handle arguments which configure the tool selector.
    {   // The tool selector is an array of buttons in the lower right corner of the control screen.
        // Configuring the tool selector is analogous to the configuration of the key bindings.
        QMap<QString, QList<KeyAction>> actions;
        QMap<QString, FullDrawTool> tools;
        actionsFromConfig(actions, tools, local, settings, "tools");

        // If no configuration for tool selector is found, send the default configuration.
        if (actions.isEmpty() && tools.isEmpty()) {
            // The default configuration contains 2 rows and 5 columns.
            ctrlScreen->getToolSelector()->setTools(2, 5, defaultToolSelectorActions, defaultToolSelectorTools);
        }
        else {
            // A new tool selector configuraton has to be created.
            // All actions need to be converted from action strings to KeyActions or FullDrawTools,
            // and the number of necessary rows and columns needs to be fixed.

            /// Map of button positions to key actions.
            QMap<quint8, QList<KeyAction>> actionMap;
            /// Map of button positions to draw tools.
            QMap<quint8, FullDrawTool> toolMap;
            /// Number of rows
            quint8 nrows = 0;
            /// Number of columns
            quint8 ncols = 0;

            // Send key actions to control screen.
            quint8 key;
            bool ok;
            // Iterate over variantMap and copy the values to inputMap, converting the keys to a quint8.
            for (QMap<QString, QList<KeyAction>>::const_iterator it = actions.cbegin(); it != actions.cend(); it++) {
                try {
                    if (it.key().length() > 2)
                        throw 0;
                    key = quint8(it.key().toUShort(&ok, 16));
                    if (!ok)
                        throw 1;
                    // Check whether nrows needs to be increased.
                    if (16*nrows <= key) // this is equivalent to nrows <= key/16 = row index of key
                        nrows = quint8(key/16) + 1; // nrows needs to be at least max(row indices) + 1, because the indices start from 0.
                    // Check whethre ncols needs to be increased.
                    if (ncols <= key%16) // key%16 is the column index of key.
                        ncols = key%16 + 1; // ncols needs to be at least max(column indices) + 1, because the indices start from 0.
                    // Initialize actionMap[key].
                    actionMap[key] = QList<KeyAction>();
                    // Iterate over all actions (refered to as "action string" in the following) defined for each key (usually this is only one).
                    for (QList<KeyAction>::const_iterator action=it->cbegin(); action!=it->cend(); action++)
                        actionMap[key].append(*action);
                } catch (int) {
                    // If the key is not understood, this (key, value) pair will be ignored.
                    qWarning() << "Could not understand index" << it.key() << "which should be a two digit integer.";
                }
            }
            // Send tools to control screen.
            for (QMap<QString, FullDrawTool>::const_iterator it = tools.cbegin(); it != tools.cend(); it++) {
                try {
                    if (it.key().length() > 2)
                        throw 0;
                    key = quint8(it.key().toUShort(&ok, 16));
                    if (!ok)
                        throw 1;
                    // Check whether nrows needs to be increased.
                    if (16*nrows <= key) // this is equivalent to nrows <= key/16 = row index of key
                        nrows = quint8(key/16) + 1; // nrows needs to be at least max(row indices) + 1, because the indices start from 0.
                    // Check whethre ncols needs to be increased.
                    if (ncols <= key%16) // key%16 is the column index of key.
                        ncols = key%16 + 1; // ncols needs to be at least max(column indices) + 1, because the indices start from 0.
                    toolMap[key] = *it;
                } catch (int) {
                    // If the key is not understood, this (key, value) pair will be ignored.
                    qWarning() << "Could not understand index" << it.key() << "which should be a two digit integer.";
                }
            }
            // Send tool selector configuration to ctrlScreen.
            ctrlScreen->getToolSelector()->setTools(nrows, ncols, actionMap, toolMap);
        }
    }


    // Set per slide times from local configuration.
    // These times will be used to change the color of the timer depending on the total time passed relative to the time set for the current frame.
    // Currently slide labels must be integers.
    if (local.contains("page times")) {
        /// Map of slide labels to times.
        QMap<int, quint32> map;
        /// QVariantMap containing all page times from the local configuration.
        QVariantMap variantMap = local["page times"].value<QVariantMap>();
        bool ok;
        quint32 key;
        // Iterate over variantMap to copy its (key, value) pairs to map.
        for (QVariantMap::const_iterator it=variantMap.cbegin(); it!=variantMap.cend(); it++) {
            // Convert the label to an integer.
            // We use integers because if only a subset of the labels is known, it should be obvious where to insert other labels.
            key = it.key().toInt(&ok);
            // Non-integer labels will be ignored and lead to a warning.
            if (!ok) {
                qCritical() << "In local config / page times: Did not understand slide number" << it.key();
                continue;
            }
            // Try to convert the value to a number of ms.
            QStringList timeStringList = it->toString().split(":");
            if (timeStringList.length() == 1)
                timeStringList = it->toString().replace(".", ":").split(":");
            unsigned int time;
            switch (timeStringList.length())
            {
            case 1:
                // Expect value in minuts, convert to ms.
                time = 60000*timeStringList[0].toUInt(&ok);
                break;
            case 2:
                // Expect value in minuts, convert to ms.
                 time = 60000*timeStringList[0].toUInt(&ok);
                 if (ok)
                    // Expect value in s, convert to ms.
                     time += 1000*timeStringList[1].toDouble(&ok);
                break;
            case 3:
                // Expect value in h, convert to ms.
                time = 3600000*timeStringList[0].toUInt(&ok);
                if (ok)
                    // Expect value in minuts, convert to ms.
                    time += 60000*timeStringList[1].toUInt(&ok);
                if (ok)
                    // Expect value in s, convert to ms.
                    time += 1000*timeStringList[2].toDouble(&ok);
                break;
            default:
                ok = false;
            }
            if (!ok) {
                qCritical() << "In local config / page times: Did not understand time" << *it;
                continue;
            }
            // Add the (page, time) pair to map.
            map.insert(key, time);
        }
        // Send the timer configuration to ctrlScreen (which sends it to the timer)
        ctrlScreen->getTimer()->setTimeMap(map);
    }



    // Simple options: just strings

    // Set presentation time.
    if (!parser.value("t").isEmpty())
        ctrlScreen->getTimer()->setString(parser.value("t"));
    else if (local.contains("time"))
        ctrlScreen->getTimer()->setString(local.value("time").toStringList()[0]);
    else if (settings.contains("time"))
        ctrlScreen->getTimer()->setString(settings.value("time").toStringList()[0]);

#ifdef EMBEDDED_APPLICATIONS_ENABLED
    // Set (list of) files, which will be executed in an embedded widget (using X embedding).
    // The files are just parsed as a string list.
    // TODO: Wildcard characters
    if (!parser.value("e").isEmpty())
        ctrlScreen->setEmbedFileList(parser.value("e").split(","));
    else if (local.contains("embed"))
        ctrlScreen->setEmbedFileList(local.value("embed").toStringList());
    else if (settings.contains("embed"))
        ctrlScreen->setEmbedFileList(settings.value("embed").toStringList());

    // Set a program, which will convert PIDs to Window IDs.
    // This can be used to determine window IDs which are required for embeddings widgets in X.
    // The value is directly parsed to ctrlScreen (except if it is "none").
    if (!parser.value("w").isEmpty()) {
        if (parser.value("w").toLower() != "none")
            ctrlScreen->setPid2WidConverter(parser.value("w"));
    }
    else if (local.contains("pid2wid")) {
        QString string = local.value("pid2wid").toString();
        if (string.toLower() != "none")
            ctrlScreen->setPid2WidConverter(string);
    }
    else if (settings.contains("pid2wid")) {
        QString string = settings.value("pid2wid").toString();
        if (string.toLower() != "none")
            ctrlScreen->setPid2WidConverter(string);
    }
#endif

    // Set the icon path.
    {   // Set the path used for the icon theme.
        // Icons are currently only used in the tool selector.
        QString icontheme = parser.value("icon-path");
        // Check whether icontheme is a directory. Reset to "" otherwise.
        if (!icontheme.isEmpty() && !QFileInfo(icontheme).isDir()) {
            qWarning() << "icon path" << icontheme << "not understood: should be a directory";
            icontheme = QString();
        }
        if (icontheme.isEmpty())
            icontheme = local.value("icon-theme", "").toString();
        // Check whether icontheme is a directory. Reset to "" otherwise.
        if (!icontheme.isEmpty() && !QFileInfo(icontheme).isDir()) {
            qWarning() << "icon path" << icontheme << "not understood: should be a directory";
            icontheme = QString();
        }
        if (icontheme.isEmpty())
            icontheme = settings.value("icon-theme", "").toString();
        if (!icontheme.isEmpty()) {
            // Set the icon theme if it is a directory.
            if (QFileInfo(icontheme).isDir()) {
                QIcon::setThemeSearchPaths({icontheme});
                QIcon::setThemeName(icontheme);
            }
            else
                qWarning() << "icon path" << icontheme << "not understood: should be a directory";
        }
    }

    // Set character, which is used to split links into a file name and arguments.
    if (!parser.value("u").isEmpty())
        ctrlScreen->setUrlSplitCharacter(parser.value("u"));
    else if (local.contains("urlsplit"))
        ctrlScreen->setUrlSplitCharacter(local.value("urlsplit").toString());
    else if (settings.contains("urlsplit"))
        ctrlScreen->setUrlSplitCharacter(settings.value("urlsplit").toString());

    // Handle settings which should have boolean values.
    {
        bool value;
        // Enable or disable caching videos.
        value = boolFromConfig(parser, local, settings, "video-cache", true);
        ctrlScreen->getPresentationSlide()->setCacheVideos(true);

        // Mute or unmute multimedia content in the presentation.
        value = boolFromConfig(parser, local, settings, "mute-presentation", false);
        ctrlScreen->getPresentationSlide()->setMuted(value);

        // Mute or unmute multimedia content on the control screen.
        value = boolFromConfig(parser, local, settings, "mute-notes", true);
        ctrlScreen->getNotesSlide()->setMuted(value);
    }

    // Handle settings that are either qreal or bool
    {
        qreal value;
        // Set autostart or delayed autostart of multimedia content.
        value = qrealFromConfig(parser, local, settings, "autoplay", -2.);
        emit ctrlScreen->setAutostartDelay(value);

#ifdef EMBEDDED_APPLICATIONS_ENABLED
        // Set autostart or delayed autostart of embedded applications.
        value = qrealFromConfig(parser, local, settings, "autostart-emb", -2.);
        ctrlScreen->getPresentationSlide()->setAutostartEmbeddedDelay(value);
#endif

        // Set minimum sidebar width for control screen.
        value = qrealFromConfig(parser, local, settings, "sidebar-width", 0.2, 1.);
        ctrlScreen->setMinSidebarWidth(value);

        // Set radius of eraser tool.
        value  = qrealFromConfig(parser, local, settings, "eraser-size", 10, 1e5);
        ctrlScreen->getPresentationSlide()->getPathOverlay()->setEraserSize(value);
    }

    // Settings with integer values
    {
        quint32 value;

        // Set minimum time per frame in animations generated by showing slides in rapid succession.
        value = intFromConfig<quint32>(parser, local, settings, "min-delay", 40);
        ctrlScreen->setAnimationDelay(value);

        // Set (estimate of) the maximum cache size in MB.
        // This is the amount of RAM which BeamerPresenter should at most use for pre-rendering slides.
        // It can happen that the cache grows beyond this size, because the size of new slides rendered
        // to cache is only known after they have been saved to cache.
        value = intFromConfig<quint32>(parser, local, settings, "memory", 200);
        ctrlScreen->setCacheSize(1048576L * value);
    }
    {
        quint8 value;

        // Set the number of blinds in blinds slide transition.
        value = intFromConfig<quint8>(parser, local, settings, "blinds", 8);
        ctrlScreen->getPresentationSlide()->setBlindsNumber(value);

        // Set the number of columns in overview mode.
        value = intFromConfig<quint8>(parser, local, settings, "columns", 5);
        if (value<1)
            qCritical() << "You tried to set a number of columns < 1. You shouldn't expect anyting reasonable!";
        else
            ctrlScreen->setOverviewColumns(value);

        // Set the number of visible TOC (tabel of contents) levels.
        // One additional level can be shown in a drop down menu
        value = intFromConfig<quint8>(parser, local, settings, "toc-depth", 2);
        if (value<1)
            qCritical() << "You tried to set a number of TOC levels < 1. You shouldn't expect anyting reasonable!";
        else
        ctrlScreen->setTocLevel(value);
    }
    {
        int value;

        // Set scroll step for touch pad input devices.
        // A larger value makes scrolling slower.
        value = intFromConfig<int>(parser, local, settings, "scrollstep", 200);
        ctrlScreen->setScrollDelta(value);

        // Set maximum number of cached slides.
        // This restricts only the number of slides which are pre-rendered to cache, not the actual amount of memory used.
        value = intFromConfig<int>(parser, local, settings, "cache", -1);
        ctrlScreen->setCacheNumber(value);
    }
    {
        quint16 value;

        // Set number of glitter steps and size of glitter pixels in glitter slide transition.
        // More details can (hopefully) be found in the man page.
        value  = intFromConfig<quint16>(parser, local, settings, "glitter-pixel", 30);
        ctrlScreen->getPresentationSlide()->setGlitterPixel(value);
        value  = intFromConfig<quint16>(parser, local, settings, "glitter-steps", 30);
        ctrlScreen->getPresentationSlide()->setGlitterSteps(value);
    }


    // Options without any arguments.

    // Disable all slide transitions.
    if (parser.isSet("d"))
        ctrlScreen->getPresentationSlide()->disableTransitions();
    else if (local.contains("no-transitions")) {
        // This is rather unintuitive. Just set any value...
        if (!QStringList({"false", "no", "transitions", "0"}).contains(local.value("no-transitions").toString().toLower()))
            ctrlScreen->getPresentationSlide()->disableTransitions();
    }
    else if (settings.contains("no-transitions"))
        ctrlScreen->getPresentationSlide()->disableTransitions();

    // Enforce treating all scroll inputs as touch pads.
    if (parser.isSet("force-touchpad"))
        ctrlScreen->setForceTouchpad();
    else if (local.contains("force-touchpad")) {
        // This is rather unintuitive. Just set any value...
        if (!QStringList({"false", "no", "0"}).contains(local.value("force-touchpad").toString().toLower()))
            ctrlScreen->setForceTouchpad();
    }
    else if (settings.contains("force-touchpad"))
        ctrlScreen->setForceTouchpad();

    // Log times of slide changes and timer value at slide changes
    if (parser.isSet("log"))
        ctrlScreen->setLogSlideChanges(true);
    else if (local.contains("log")) {
        // This is rather unintuitive. Just set any value...
        if (!QStringList({"false", "no", "no log", "no-log", "0"}).contains(local.value("log").toString().toLower()))
            ctrlScreen->setLogSlideChanges(true);
    }
    else if (settings.contains("log"))
        ctrlScreen->setLogSlideChanges(true);


    // Settings, which can cause exceptions

    // Set a custom renderer.
    try {
        if (parser.value("r").isEmpty()) {
            // No option given on the command line.
            // This is not an error, but formally treated as an exception here.
            // TODO: don't treat this as an exception.
            throw 0;
        }
        else if (parser.value("r") == "custom") {
            // In this case a custom renderer should be found in the local or global configuration.
            throw -1;
        }
        else if (parser.value("r") != "poppler") {
            // This really defines a custom renderer. Try to parse it to ctrlScreen.
            ctrlScreen->setRenderer(parser.value("r").split(" "));
            // ctrlScreen->setRenderer can throw exceptions > 0.
        }
    }
    catch (int const err) {
        // This is executed if no custom renderer has been set yet.
        if (err > 0) {
            // Show error message for exceptions in ctrlScreen->setRenderer.
            qWarning() << "Failed to set custom renderer";
        }
        try {
            // Check for custom renderer in local configuration.
            if (!local.contains("renderer")) {
                // If err==-1: throw -1 because this means that we expect a definition of a renderer in the global configuration.
                // Otherwise: throw 0, because error messages have already been shown if necessary.
                throw err>0 ? 0 : err;
            }
            ctrlScreen->setRenderer(local.value("renderer").toString().split(" "));
            // This can agan throw exceptions > 0
        }
        catch (int const err) {
            // This is executed if no custom renderer has been set yet.
            if (err > 0) {
                // Show error message for exceptions in ctrlScreen->setRenderer.
                qWarning() << "Failed to set custom renderer from local configuration file";
            }
            if (settings.contains("renderer")) {
                try {
                    // Check for custom renderer in global configuration.
                    ctrlScreen->setRenderer(settings.value("renderer").toString().split(" "));
                }
                catch (int const) {
                    // exception thrown by ctrlScreen
                    qWarning() << "Failed to set custom renderer from configuration";
                }
            }
            if (err==-1) {
                // This message is show if the renderer was set to "custom", but no custom renderer was defined.
                qWarning() << "Ignored request to use undefined custom renderer";
            }
        }
    }


    // Decide whether ctrlScreen should be shown depending on arguments, settings and the QPA backend.
    // Usually checking the QPA backend is not necessary. It can therefore be switched off.
#ifdef CHECK_QPA_PLATFORM
    {
        // Check whether the notes should be shown (options "no-notes" or "n" and "force-show").
        bool showNotes = !parser.isSet("n");
        if (showNotes && !parser.isSet("force-show")) {
            if (local.contains("no-notes")) {
                // This is rather unintuitive. Just set any value...
                QString string = local.value("no-notes").toString().toLower();
                if (QStringList({"", "true", "no-notes", "1"}).contains(string))
                    showNotes = false;
            }
            else if (settings.contains("no-notes"))
                showNotes = false;
        }

        // Check the QPA backend and show ctrlScreen.
        try {
            // TODO: Checks at compile time instead of execution time.
            if (!QStringList({"xcb", "wayland", "wayland-egl", "windows"}).contains(app.platformName())) {
                // Using an untested platform. Show some information to the user.
                if (QStringList({"wayland-xcomposite-egl", "wayland-xcomposite-glx", "direct2d", "cocoa", "haiku"}).contains(app.platformName())) {
                    // No problems expected.
                }
                else if (QStringList({"qnx", "android", "iOS"}).contains(app.platformName())) {
                    // Problems could occur.
                    qWarning() << "BeamerPresenter might cause problems on your platform.";
                    if (showNotes)
                        qInfo() << "You can use the option -n or --no-notes to avoid showing to windows.";
                }
                else if (QStringList({"minimal", "minimalegl", "offscreen", "vnc", "mirclient", "winrt"}).contains(app.platformName())) {
                    // Problems expected. Don't show two windows by default.
                    if (parser.isSet("force-show")) {
                        qWarning() << "Your QPA platform plugin might cause problems when trying to show two windows.";
                    }
                    else {
                        qWarning() << "Your QPA platform might cause problems when trying to show two windows. Showing only the presentation window.";
                        qInfo() << "You can force showing notes with the option --force-show";
                        throw;
                    }
                }
                else if (QStringList({"eglfs", "linuxfb", "directfb", "kms", "openwfd", "bsdfb"}).contains(app.platformName())) {
                    // Serious problems expected. Exit with an error by default.
                    qCritical() << "You are using a QPA platform plugin, on which BeamerPresenter might freeze and block your system.";
                    qInfo() << "QPA platform plugin is:" << app.platformName();
                    if (parser.isSet("force-show")) {
                        qWarning() << "Continuing anyway, because you have set --force-show";
                        throw;
                    }
                    else {
                        qInfo() << "Exiting now to avoid crashing you system. You can force showing the presentation with --force-show.";
                        return 1;
                   }
                }
                else {
                    qWarning() << "You are using an unknown QPA platform plugin.";
                    if (showNotes)
                        qInfo() << "If your platform (plugin) is not able to show two windows, please use the option -n or --no-notes.";
                }
                qInfo() << "BeamerPresenter has not been tested on a similar platform. Your feedback is very welcome.";
                qInfo() << "Using QPA platform plugin" << app.platformName();
            }
            if (showNotes) {
                // Show ctrlScreen.
                ctrlScreen->show();
                ctrlScreen->activateWindow();
            }
        }
        catch (...) {}
    }
#else
    // Check whether the notes should be shown (option "no-notes" or "n").
    if ( !(
            parser.isSet("n")
            || (local.contains("no-notes") && QStringList({"", "true", "no-notes", "1"}).contains(local.value("no-notes").toString().toLower()))
            || settings.contains("no-notes")
        ) ) {
        // Show ctrlScreen.
        ctrlScreen->show();
        ctrlScreen->activateWindow();
    }
#endif

    // Render the first page on presentation screen. Do not set a frame time for this first slide.
    emit ctrlScreen->sendNewPageNumber(0, false);
    // Render the first page on control screen.
    ctrlScreen->renderPage(0);

    // Load drawings if a BeamerPresenter drawings file is given.
    // The drawings file can be handed to BeamerPresenter as single positional argument, but is then internally written to the local configuration.
    if (local.contains("drawings")) {
        QString drawpath = local.value("drawings").toString();
        // Load the drawings.
        ctrlScreen->loadXML(drawpath);
    }

    // Start the execution loop.
    int status = app.exec();
    // Tidy up and exit.
    delete ctrlScreen;
    return status;
}
