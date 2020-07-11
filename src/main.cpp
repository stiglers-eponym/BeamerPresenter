#include <QApplication>
#include <QSettings>
#include <QCommandLineParser>
#include <QFileDialog>
#include <src/enumerates.h>
#include "src/preferences.h"
#include "src/master.h"


int main(int argc, char *argv[])
{
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

    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "beamerpresenter-new", "beamerpresenter-new");
    Preferences& wpreferences = writable_preferences();
    if (settings.contains("gui config"))
        wpreferences.gui_config_file = settings.value("gui config").toString();

    Master master;
    if (parser.positionalArguments().isEmpty())
    {
        master.addFile("presentation", QFileDialog::getOpenFileName(nullptr, "Presentation file", "", "Documents (*.pdf)"));
    }
    else
    {
        master.addFile("presentation", parser.positionalArguments().first());
    }

    master.readGuiConfig(preferences().gui_config_file);
    master.showAll();

    int status = app.exec();

    return status;
}
