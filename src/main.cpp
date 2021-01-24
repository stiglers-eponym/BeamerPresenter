#include <QApplication>
#include <QSettings>
#include <QCommandLineParser>
#include <src/enumerates.h>
#include "src/preferences.h"
#include "src/master.h"

#ifdef INCLUDE_POPPLER
// TODO: make this failsafe (poppler-version.h is not available in early
// versions of Poppler)
#include <poppler/qt5/poppler-version.h>
#endif
#ifdef INCLUDE_MUPDF
#include <mupdf/fitz/version.h>
#endif

/// Provides globally available pointer to writable preferences.
Preferences &writable_preferences(Preferences *new_preferences)
{
    static Preferences *preferences{new_preferences};
    return *preferences;
}

/// Get read-only globally shared preferences object.
/// This is the usual way of accessing preferences.
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
    parser.setApplicationDescription("\nModular multi screen PDF presenter");

    // Define command line options.
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    parser.addOption({{"c", "config"}, "settings / configuration file", "file"});
    parser.addOption({{"g", "gui-config"}, "user interface configuration file", "file"});
    parser.addOption({{"t", "time"}, "timer total time in minutes", "number"});
    parser.addOption({{"p", "page-part"}, "interpret half of pages as presentation, other as notes", "side"});
#if defined(INCLUDE_MUPDF) and defined(INCLUDE_POPPLER)
    parser.addOption({"renderer", "PDF renderer: MuPDF/poppler/external-MuPDF/external-poppler", "name"});
#elif defined(INCLUDE_MUPDF)
    parser.addOption({"renderer", "PDF renderer: external or MuPDF", "name"});
#elif defined(INCLUDE_POPPLER)
    parser.addOption({"renderer", "PDF renderer: external or poppler", "name"});
#endif
    parser.process(app);

    {
        // Initialize preferences
        Preferences *wpreferences;
        if (parser.isSet("c"))
            wpreferences = new Preferences(parser.value("c"));
        else
            wpreferences = new Preferences();
        writable_preferences(wpreferences);
        wpreferences->loadSettings();
        wpreferences->loadFromParser(parser);
    }

    Master master;
    if (!master.readGuiConfig(parser.value("g").isEmpty() ? preferences().gui_config_file : parser.value("g")))
    {
        qCritical() << "Parsing the GUI configuration failed. Probably the GUI config is unavailable or invalid or no valid PDF files were found.";
        delete &preferences();
        parser.showHelp(-1);
        // This quits the program and returns exit code -1.
    }
    master.showAll();
    master.navigateToPage(0);
    master.distributeMemory();
    QObject::connect(&preferences(), &Preferences::distributeMemory, &master, &Master::distributeMemory);
    const int status = app.exec();
    delete &preferences();
    return status;
}
