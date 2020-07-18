#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMap>
#include <QColor>

/// Does a single PDF include both presentation and notes?
/// In this case PagePart shows which part is currently of interest.
enum PagePart
{
    FullPage = 0,
    LeftHalf = 1,
    RightHalf = -1,
};

/// Page shifts are stored as integers in SlideScenes.
/// The information about whether overlays should be considered is stored in
/// the bits controlled by FirstOverlay and LastOverlay.
enum ShiftOverlays
{
    NoOverlay = 0,
    FirstOverlay = (INT_MAX >> 2) + 1,
    LastOverlay = (INT_MAX >> 1) + 1,
    AnyOverlay = (LastOverlay | FirstOverlay),
};

enum LinkType
{
    NoLink = -1,
    NavigationLink = -2,
    ExternalLink = -3,
    MovieLink = -4,
    SoundLinx = -5,
};

enum Action
{
    InvalidAction,
    NoAction,
    Update,
    NextPage,
    PreviousPage,
    NextSkippingOverlays,
    PreviousSkippingOverlays,
    FirstPage,
    LastPage,
    ReloadFiles,
    Quit,
};

enum BasicTool {
    NoTool,
};

#endif // ENUMERATES_H
