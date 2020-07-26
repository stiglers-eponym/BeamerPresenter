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

/// Types of links in PDF.
/// These are all negative, because positive values are interpreted as page
/// numbers for internal navigation links.
enum LinkType
{
    NoLink = -1,
    NavigationLink = -2,
    ExternalLink = -3,
    MovieLink = -4,
    SoundLinx = -5,
};

/// Actions triggered by keyboard shortcuts or buttons.
enum Action
{
    InvalidAction,
    NoAction,
    // Nagivation actions
    Update,
    NextPage,
    PreviousPage,
    NextSkippingOverlays,
    PreviousSkippingOverlays,
    FirstPage,
    LastPage,
    // Other actions
    ReloadFiles,
    Quit,
};

/// Tools for drawing and highlighting.
enum BasicTool {
    NoTool,
};

#endif // ENUMERATES_H
