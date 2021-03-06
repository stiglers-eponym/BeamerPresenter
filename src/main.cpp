#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QCommandLineParser>
#include <src/enumerates.h>
#include "src/preferences.h"
#include "src/master.h"

#ifdef INCLUDE_POPPLER
// TODO: make this failsafe (poppler-version.h is not available in early
// versions of Poppler)
#if __has_include(<poppler/qt5/poppler-version.h>)
#include <poppler/qt5/poppler-version.h>
#else
#define POPPLER_VERSION "OLD"
#endif
#endif
#ifdef INCLUDE_MUPDF
extern "C"
{
#include <mupdf/fitz/version.h>
}
#endif

/// Provides globally available pointer to writable preferences.
Preferences *writable_preferences(Preferences *new_preferences)
{
    static Preferences *preferences{new_preferences};
    return preferences;
}

/// Get read-only globally shared preferences object.
/// This is the usual way of accessing preferences.
const Preferences *preferences()
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
    app.setWindowIcon(QIcon(ICON_FILEPATH));

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
    parser.addOption({"log", "log slide changes to standard output"});
#ifdef QT_DEBUG
    parser.addOption({"debug", "debug flags, comma-separated", "flags"});
#endif
#if defined(INCLUDE_MUPDF) and defined(INCLUDE_POPPLER)
    parser.addOption({"renderer", "PDF renderer: MuPDF / poppler / external-MuPDF / external-poppler", "name"});
#elif defined(INCLUDE_MUPDF)
    parser.addOption({"renderer", "PDF renderer: external or MuPDF", "name"});
#elif defined(INCLUDE_POPPLER)
    parser.addOption({"renderer", "PDF renderer: external or poppler", "name"});
#endif
    parser.process(app);

    {
        // Initialize preferences.
        Preferences *wpreferences;
        if (parser.isSet("c"))
            // parser.value("c") should be the name of a configuration file.
            // If that does not exist or cannot be read, the program can start
            // anyway, if a valid gui config file is defined in parser.value("g").
            wpreferences = new Preferences(parser.value("c"));
        else
            wpreferences = new Preferences();
        writable_preferences(wpreferences);
#ifdef QT_DEBUG
        wpreferences->loadDebugFromParser(parser);
#endif
        wpreferences->loadSettings();
        wpreferences->loadFromParser(parser);
    }

    Master *master = new Master();
    {
        // Create the user interface.
        const char status = master->readGuiConfig(parser.value("g").isEmpty() ? preferences()->gui_config_file : parser.value("g"));
        if (status)
        {
            // Creating GUI failed. Check status and try to load default GUI config.
            // status == 4 indicates that only loading PDF files failed. In this case
            // the GUI config should not be reloaded.
            if (status < 4 && master->readGuiConfig(DEFAULT_GUI_CONFIG_PATH) == 0)
                qWarning() << "Using fallback GUI config file" << DEFAULT_GUI_CONFIG_PATH;
            else
            {
                qCritical() << "Parsing the GUI configuration failed. Probably the GUI config is unavailable or invalid or no valid PDF files were found.";
                delete master;
                delete preferences();
                // Show help and exit.
                parser.showHelp(status);
            }
        }
    }
    // Show all windows.
    master->showAll();
    // Navigate to first page.
    master->navigateToPage(0);
    // Distribute cache memory.
    master->distributeMemory();
    QObject::connect(preferences(), &Preferences::distributeMemory, master, &Master::distributeMemory);
    // Run the program.
    const int status = app.exec();
    // Clean up. preferences() must be deleted after everything else.
    // deleting master may take some time since this requires the interruption
    // and deletion of multiple threads.
    delete master;
    delete preferences();
    return status;
}
