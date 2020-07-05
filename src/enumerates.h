#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMap>
#include <QColor>

/// Does a single PDF include both presentation and notes?
/// In this case PagePart shows which part is currently of interest.
enum PagePart {
    FullPage = 0,
    LeftHalf = 1,
    RightHalf = -1,
};

enum KeyAction {
    NoAction
};

enum BasicTool {
    NoTool
};

#endif // ENUMERATES_H
