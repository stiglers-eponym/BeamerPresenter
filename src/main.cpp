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
    QCommandLineParser parser;
    parser.process(app);
    if (parser.positionalArguments().size() != 2) {
        std::cerr << "This function requires exactly 2 positional arguments." << std::endl;
        return 1;
    }
    ControlScreen w(parser.positionalArguments().at(0), parser.positionalArguments().at(1));
    w.show();
    w.renderPage(0);
    emit w.sendNewPageNumber(0);

    return app.exec();
}
