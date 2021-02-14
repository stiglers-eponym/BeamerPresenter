#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMap>
#include <QColor>
#include <QDebug>

/**
 * Log level: combinable debug or log flags
 */
enum LogLevel
{
    NoLog = 0,
    LogSlideChanges = 1 << 0,
    DebugRendering = 1 << 3,
    DebugCache = 1 << 4,
    DebugDrawing = 1 << 5,
    DebugMedia = 1 << 6,
    DebugKeyInput = 1 << 7,
    DebugSettings = 1 << 8,
    DebugTransitions = 1 << 9,
    DebugPageChange = 1 << 10,
    DebugLayout = 1 << 11,
    DebugWidgets = 1 << 12,
    DebugAll = 0x7ffe,
    DebugVerbose = 1 << 15,
};

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
Q_DECLARE_METATYPE(Action);

/// Tools for drawing and highlighting.
enum BasicTool
{
    // Invalid
    InvalidTool = 0,
    // Draw tools: first 4 bits, class DrawTool
    Pen = 1 << 0,
    FixedWidthPen = 2 << 0,
    Highlighter = 3 << 0,
    Eraser = 4 << 0,
    AnyDrawTool = 0x0f << 0,
    // Highlighting tools: next 4 bits, class PointingTool
    Pointer = 1 << 4,
    Torch = 2 << 4,
    Magnifier = 3 << 4,
    AnyPointingTool = 0x0f << 4,
    // Other tools, class Tool
    TextInputTool = 1 << 8,
    // No tool
    NoTool = 1 << 11,
};

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

/**
 * Obtain Qt::MouseButton by taking InputDevice >> 1.
 */
enum InputDevice
{
    NoDevice = 0,
    MouseNoButton = 1,
    MouseLeftButton = Qt::LeftButton << 1,
    MouseRightButton = Qt::RightButton << 1,
    MouseMiddleButton = Qt::MiddleButton << 1,
    TabletPen = 1 << 4,
    TabletEraser = 1 << 5,
    TabletCursor = 1 << 6,
    TabletOther = 1 << 7,
    TabletNoPressure = 1 << 8,
    TouchInput = 1 << 9,
    AnyDevice = 0xffff,
    AnyPointingDevice = AnyDevice ^ (TabletEraser | MouseRightButton | MouseMiddleButton),
    AnyNormalDevice = AnyPointingDevice ^ (TabletNoPressure | MouseNoButton),
};

enum OverlayDrawingMode
{
    PerPage, // Every page has independent drawings.
    PerLabel, // All pages with the same label in a simply connected region have the same drawings.
    Cumulative, // When going to the next page which has the same label, the current drawings are copied.
};

#endif // ENUMERATES_H
