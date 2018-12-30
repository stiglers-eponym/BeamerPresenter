/*
 * This file is part of BeamerPresent.
 *
 * BeamerPresent is free and unencumbered public domain software.
 * For more information, see http://unlicense.org/ or the accompanying
 * UNLICENSE file.
 */

#include <iostream>
#include <QApplication>
#include <QCommandLineParser>
#include "controlscreen.h"
#include "presentationscreen.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("beamerpresenter");

    QCommandLineParser parser;
    parser.setApplicationDescription(
            "\nSimple dual screen pdf presentation software.\n"
            "Shortcuts:\n"
            "  q                quit\n"
            "  g                go to page (set focus to page number edit)\n"
            "  p                pause / continue timer\n"
            "  r                reset timer\n"
            "  space            update layout and start timer\n"
            "  Up / Down        go to previous / next slide\n"
            "  Left / Right     go to previous / next slide\n"
            "  PageUp/PageDown  go to previous / next slide and start timer"
        );
    parser.addHelpOption();
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    parser.addPositionalArgument("<notes.pdf>",  "Notes for the presentation\n(should have the same number of pages as <slides.pdf>)");
    parser.addOptions({
        {{"t", "timer"}, "set timer to <time>.\nPossible formats are \"[m]m\", \"[m]m:ss\" and \"h:mm:ss\".", "time"},
    });
    parser.process(app);
    if (parser.positionalArguments().size() != 2)
        parser.showHelp(1);
    ControlScreen w(parser.positionalArguments().at(0), parser.positionalArguments().at(1));
    w.show();
    w.renderPage(0);
    emit w.sendNewPageNumber(0);
    if ( !parser.value("t").isEmpty() )
        emit w.sendTimerString(parser.value("t"));

    return app.exec();
}
