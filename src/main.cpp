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
            "  q                       quit\n"
            "  g                       go to page (set focus to page number edit)\n"
            "  p                       pause / continue timer\n"
            "  r                       reset timer\n"
            "  o                       toggle cursor visbility (only on presentation screen)\n"
            "  m                       play or pause all multimedia content\n"
            "  space                   update layout and start timer or continue\n"
            "  Left, Up, PageUp        go to previous slide and start or continue timer\n"
            "  Right, Down, PageDown   go to next slide and start or continue timer\n"
            "  F11, f                  toggle fullscreen (only for current window)\n"
            "\n"
            "This program has experimental support for videos.\n"
            "Many options which can be provided in a pdf are not supported by this program.\n"
        );
    parser.addHelpOption();
    parser.addPositionalArgument("<slides.pdf>", "Slides for a presentation");
    parser.addPositionalArgument("<notes.pdf>",  "Notes for the presentation (optional, should have the same number of pages as <slides.pdf>)");
    parser.addOptions({
        {{"t", "timer"}, "set timer to <time>.\nPossible formats are \"[m]m\", \"[m]m:ss\" and \"h:mm:ss\".", "time"},
        {{"a", "autoplay"}, "true, false or number: start video and audio content when entering a slide.\nA number is interpreted as a delay in seconds, after which multimedia content is started.", "value"},
        {{"m", "min-delay"}, "set minimum time per frame in milliseconds.\nThis is useful when using \\animation in LaTeX beamer.", "ms"},
        {{"d", "tolerance"}, "tolerance for the presentation time in seconds.\nThe timer will be white <secs> before the timeout, green when the timeout is reached, yellow <secs> after the timeout and red 2*<secs> after the timeout.", "secs"},
    });

    parser.process(app);
    ControlScreen * w;
    if (parser.positionalArguments().size() == 1)
        w = new ControlScreen(parser.positionalArguments().at(0));
    else if (parser.positionalArguments().size() == 2)
        w = new ControlScreen(parser.positionalArguments().at(0), parser.positionalArguments().at(1));
    else
        parser.showHelp(1);

    // Set tolerance for presentation time
    if ( !parser.value("d").isEmpty() ) {
        bool success;
        int tolerance = parser.value("d").toInt(&success);
        if (success)
            emit w->sendTimeoutInterval(tolerance);
        else
            std::cerr << "option \"" << parser.value("d").toStdString() << "\" to tolerance not understood." << std::endl;
    }

    // Set presentation time
    if ( !parser.value("t").isEmpty() )
        emit w->sendTimerString(parser.value("t"));

    // Set minimum time per frame
    if ( !parser.value("m").isEmpty() ) {
        bool success;
        int delay = parser.value("m").toInt(&success);
        if (success)
            emit w->sendAnimationDelay(delay);
        else
            std::cerr << "option \"" << parser.value("m").toStdString() << "\" to min-delay not understood." << std::endl;
    }

    // Set autostart or delay for multimedia content
    if ( !parser.value("a").isEmpty() ) {
        double delay = 0.;
        bool success;
        QString a = parser.value("a").toLower();
        delay = a.toDouble(&success);
        if (success)
            emit w->sendAutostartDelay(delay);
        else if (a == "true")
            emit w->sendAutostartDelay(0.);
        else if (a == "false")
            emit w->sendAutostartDelay(-2.);
        else
            std::cerr << "option \"" << parser.value("a").toStdString() << "\" to autoplay not understood." << std::endl;
    }

    w->show();
    // Render first page on presentation screen
    emit w->sendNewPageNumber(0);
    // Render first page on control screen
    w->renderPage(0);
    // Here one could update the cache.
    // But you probably first want to adjust the window size and then update it with key shortcut space.
    //emit w->sendUpdateCache();
    //w->updateCache();
    int status = app.exec();
    delete w;
    return status;
}
