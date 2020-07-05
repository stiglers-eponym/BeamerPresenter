#include <QApplication>
#include <QSettings>
#include <QCommandLineParser>
#include <src/enumerates.h>


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
            "\nDual screen PDF presentation\n"
            );

    // Define command line options.
    parser.addHelpOption();
    parser.addVersionOption();

    // TODO: more positional arguments
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");

    // TODO: change letters for option shortcuts
    //parser.addOptions({});

    ControlScreen *ctrlScreen;
    ctrlScreen = new ControlScreen(presentation);
    ctrlScreen->show()

    int status = app.exec();

    delete ctrlScreen;
    return status;
}
