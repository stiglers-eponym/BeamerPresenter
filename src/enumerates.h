#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMap>
#include <QColor>
#include <QDebug>

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
    ScrollDown,
    ScrollUp,
    ScrollNormal,
    SaveDrawings,
    SaveDrawingsAs,
    LoadDrawings,
    LoadDrawingsNoClear,
    // Timer
    StartTimer,
    StopTimer,
    StartStopTimer,
    ResetTimePassed,
    // Media
    PlayMedia,
    PauseMedia,
    PlayPauseMedia,
    // Other actions
    ReloadFiles,
    FullScreen,
    Quit,
    QuitNoConfirmation,

    // Combinations with other types
    UndoDrawingLeft = UndoDrawing | PagePart::LeftHalf,
    UndoDrawingRight = UndoDrawing | PagePart::RightHalf,
    RedoDrawingLeft = RedoDrawing | PagePart::LeftHalf,
    RedoDrawingRight = RedoDrawing | PagePart::RightHalf,
    ClearDrawingLeft = ClearDrawing | PagePart::LeftHalf,
    ClearDrawingRight = ClearDrawing | PagePart::RightHalf,
};
Q_DECLARE_METATYPE(Action);

enum GuiWidget
{
    InvalidType = 0, // QWidget
    VBoxWidgetType, // ContainerWidget, QBoxLayout
    HBoxWidgetType, // ContainerWidget, QBoxLayout
    StackedWidgetType, // StackedWidget
    TabbedWidgetType, // QTabWidget
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

#endif // ENUMERATES_H
