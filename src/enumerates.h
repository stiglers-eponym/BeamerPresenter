#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMap>
#include <QColor>

/// If a single PDF includes both presentation and notes,
/// PagePart shows which part is currently of interest.
/// The numbers are chosen such that (page number | page part)
/// can be used to label pages including the page part by a single int.
enum PagePart
{
    FullPage = 0,
    LeftHalf = (INT_MAX >> 2) + 1,
    RightHalf = (INT_MAX >> 1) + 1,
    NotFullPage = (LeftHalf | RightHalf),
};

/// Page shifts are stored as integers in SlideScenes.
/// The information about whether overlays should be considered is stored in
/// the bits controlled by FirstOverlay and LastOverlay.
/// If shift is an int and overlay is of type ShiftOverlays:
/// shift_overlay = (shift & ~AnyOverlay) | overlay
/// overlay = shift & AnyOverlay
/// shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
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
    InvalidAction = 0,
    NoAction,
    // Nagivation actions
    Update,
    NextPage,
    PreviousPage,
    NextSkippingOverlays,
    PreviousSkippingOverlays,
    FirstPage,
    LastPage,
    // Drawing
    UndoDrawing,
    RedoDrawing,
    ClearDrawing,
    // Timer
    StartTimer,
    StopTimer,
    ResetTimePassed,
    // Other actions
    ReloadFiles,
    Quit,

    // Combinations with other types
    UndoDrawingLeft = UndoDrawing | PagePart::LeftHalf,
    UndoDrawingRight = UndoDrawing | PagePart::RightHalf,
    RedoDrawingLeft = RedoDrawing | PagePart::LeftHalf,
    RedoDrawingRight = RedoDrawing | PagePart::RightHalf,
    ClearDrawingLeft = ClearDrawing | PagePart::LeftHalf,
    ClearDrawingRight = ClearDrawing | PagePart::RightHalf,
};

/// Tools for drawing and highlighting.
enum BasicTool
{
    InvalidTool = 0,
    NoTool,
    Pen,
    FixedWidthPen,
    Eraser,
    Highlighter,
    Pointer,
    Torch,
    Magnifier,
};

enum GuiWidget
{
    InvalidType = 0, // QWidget
    VBoxWidgetType, // ContainerWidget, QBoxLayout
    HBoxWidgetType, // ContainerWidget, QBoxLayout
    StackedWidgetType, // StackedWidget
    TabedWidgetType, // QTabWidget
    SlideType, // SlideView (QGraphicsView)
    OverviewType,
    TOCType,
    NotesType,
    ToolSelectorType,
    SettingsType,
    ClockType,
    TimerType,
    SlideNumberType,
    SlideLabelType,
};

enum InputDevice
{
    MouseLeftButton = 1 << 0,
    MouseRightButton = 1 << 1,
    MouseMiddleButton = 1 << 2,
    TabletPen = 1 << 3,
    TabletEraser = 1 << 4,
    TabletCursor = 1 << 5,
    TabletOther = 1 << 6,
    TouchInput = 1 << 7,
    AnyDevice = 0xffff,
};

#endif // ENUMERATES_H
