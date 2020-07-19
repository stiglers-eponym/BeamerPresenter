#include <QApplication>
#include <QSettings>
#include <QCommandLineParser>
#include <src/enumerates.h>
#include "src/preferences.h"
#include "src/master.h"

// TODO: make this failsafe (poppler-version.h is not available in early
// versions of Poppler)
#ifdef INCLUDE_POPPLER
#include <poppler/qt5/poppler-version.h>
#endif
#ifdef INCLUDE_MUPDF
#include <mupdf/fitz/version.h>
#endif

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
    QString version_string = APP_VERSION " ";
#ifdef INCLUDE_POPPLER
    version_string += " poppler=" POPPLER_VERSION;
#endif
#ifdef INCLUDE_MUPDF
    version_string += " mupdf=" FZ_VERSION;
#endif
    version_string += " Qt=" QT_VERSION_STR;
#ifdef QT_DEBUG
    version_string += " debugging";
#endif
    app.setApplicationVersion(version_string);

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
    master.distributeMemory();

    int status = app.exec();

    return status;
}
