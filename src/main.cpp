// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#include <QApplication>
#include <QSettings>
#include <QIcon>
#include <QCommandLineParser>
#include "src/config.h"
#include "src/log.h"
#include "src/preferences.h"
#include "src/master.h"
#include "src/rendering/pngpixmap.h"

#ifdef USE_TRANSLATIONS
#include <QTranslator>
#endif

#ifdef USE_POPPLER
#if __has_include(<poppler/qt5/poppler-version.h>)
#include <poppler/qt5/poppler-version.h>
#else
#define POPPLER_VERSION "OLD"
#endif
#endif
#ifdef USE_MUPDF
extern "C"
{
#include <mupdf/fitz/version.h>
}
#endif

/// Provides globally available pointer to writable preferences.
/// @return globally shared preferences, writable
Preferences *writable_preferences(Preferences *new_preferences)
{
    static Preferences *preferences{new_preferences};
    return preferences;
}

/// Provides globally available pointer to preferences.
/// @return globally shared preferences, read only
const Preferences *preferences()
{
    return writable_preferences();
}

int main(int argc, char *argv[])
{
    // Set format for debugging output, warnings etc.
    // To overwrite this you can set the environment variable QT_MESSAGE_PATTERN.
    qSetMessagePattern("%{time process} %{if-debug}D%{endif}%{if-info}INFO%{endif}%{if-warning}WARNING%{endif}%{if-critical}CRITICAL%{endif}%{if-fatal}FATAL%{endif}%{if-category} %{category}%{endif} %{file}:%{line} - %{message}%{if-fatal} from %{backtrace [depth=3]}%{endif}");
    // Register meta types (required for connections).
    qRegisterMetaType<const PngPixmap*>("const PngPixmap*");
    qRegisterMetaType<Tool*>("Tool*");

    // Set up the application.
    QApplication app(argc, argv);
    app.setApplicationName("BeamerPresenter");
    app.setWindowIcon(QIcon(ICON_FILEPATH));

#ifdef USE_TRANSLATIONS
    QTranslator translator;
    if (translator.load(QLocale(), "", "", TRANSLATION_PATH))
        app.installTranslator(&translator);
#endif

    // Set app version. The string APP_VERSION is defined in src/config.h.
    app.setApplicationVersion(
                APP_VERSION
#ifdef USE_POPPLER
                " poppler=" POPPLER_VERSION
#endif
#ifdef USE_MUPDF
                " mupdf=" FZ_VERSION
#endif
                " Qt=" QT_VERSION_STR
#ifdef QT_DEBUG
                " debugging"
#endif
            );

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
    parser.addOption({"nocache", "disable cache"});
    parser.addOption({"renderer", "available PDF renderers:"
#ifdef USE_MUPDF
                      " MuPDF"
#endif
#ifdef USE_POPPLER
                      " Poppler"
#endif
#ifdef USE_QTPDF
                      " QtPDF"
#endif
#ifdef USE_EXTERNAL_RENDERER
#ifdef USE_MUPDF
                      " external-MuPDF"
#endif
#ifdef USE_POPPLER
                      " external-Poppler"
#endif
#ifdef USE_QTPDF
                      " external-QtPDF"
#endif
#endif // USE_EXTERNAL_RENDERER
                      ,
                      "name"});
#ifdef QT_DEBUG
    parser.addOption({"debug", "debug flags, comma-separated", "flags"});
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
        const QString gui_config_file = parser.value("g").isEmpty() ? preferences()->gui_config_file : parser.value("g");
        const char status = master->readGuiConfig(gui_config_file);
        if (status)
        {
            // Creating GUI failed. Check status and try to load default GUI config.
            // status == 4 indicates that only loading PDF files failed. In this case
            // the GUI config should not be reloaded.
            if (status < 4 && master->readGuiConfig(DEFAULT_GUI_CONFIG_PATH) == 0)
                preferences()->showErrorMessage(
                            Master::tr("Error while loading GUI config"),
                            Master::tr("Loading GUI config file failed for filename \"")
                            + gui_config_file
                            + Master::tr("\". Using fallback GUI config file ")
                            + DEFAULT_GUI_CONFIG_PATH);
            else
            {
                qCritical() << "Parsing the GUI configuration failed. Probably the GUI config is unavailable or invalid, or no valid PDF files were found.";
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
