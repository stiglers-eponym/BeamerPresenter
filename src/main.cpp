#include <QApplication>
#include <QSettings>
#include <QCommandLineParser>
#include <src/enumerates.h>
#include "src/preferences.h"
#include "src/master.h"

/// Provides globally available writable reference to preferences.
Preferences &writable_preferences()
{
    static Preferences preferences;
    return preferences;
}

/// Provides globally available const reference to preferences.
const Preferences &preferences()
{
    return writable_preferences();
}

int main(int argc, char *argv[])
{
    // Set format for debugging output, warnings etc.
    // To overwrite this you can set the environment variable QT_MESSAGE_PATTERN.
    qSetMessagePattern("%{time process} %{if-debug}D%{endif}%{if-info}INFO%{endif}%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}%{if-category} %{category}%{endif} %{file}:%{line} - %{message}%{if-fatal} from %{backtrace [depth=3]}%{endif}");

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
#else // QT_DEBUG
#ifdef POPPLER_VERSION
    app.setApplicationVersion(APP_VERSION " (poppler=" POPPLER_VERSION ", Qt=" QT_VERSION_STR ")");
#else
    app.setApplicationVersion(APP_VERSION " (Qt=" QT_VERSION_STR ")");
#endif
#endif // not QT_DEBUG

    // Set up command line argument parser.
    QCommandLineParser parser;
    parser.setApplicationDescription(
            "\nModular multi screen PDF presenter\n"
            );

    // Define command line options.
    parser.addHelpOption();
    parser.addVersionOption();

    // TODO: more positional arguments
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    //parser.addOptions({});
    parser.process(app);

    Preferences& wpreferences = writable_preferences();
    wpreferences.loadSettings();
    wpreferences.loadFromParser(parser);

    Master master;
    master.readGuiConfig(preferences().gui_config_file);
    master.showAll();
    emit master.navigationSignal(0);

    int status = app.exec();

    return status;
}
