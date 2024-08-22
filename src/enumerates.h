// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef ENUMERATES_H
#define ENUMERATES_H

#include <QMetaType>
#include <climits>

#include "src/config.h"

/**
 * @file
 *
 * Global enumerators
 *
 * Enumerators defined in this file should be available globally.
 */

/**
 * @brief flag added to page number to select only left or right half of PDF
 * page.
 *
 * A single PDF may include both presentation and notes, by splitting each
 * page in a left and right half. To refer to the left or right half
 * we use (page_number | page_part) where page_number is a non-negative
 * integer and page_part is as defined in this enumerator.
 */
enum PagePart {
  /// Page is not split.
  FullPage = 0,
  /// Left half of page.
  LeftHalf = (INT_MAX >> 2) + 1,
  /// Right half of page
  RightHalf = (INT_MAX >> 1) + 1,
  /// LeftHalf | RightHalf
  NotFullPage = (LeftHalf | RightHalf),
};

/// PDF engine
enum PdfEngine {
#ifdef USE_QTPDF
  /// Internal Qt PDF engine (Qt PDF)
  QtPDFEngine = 0,
#endif
#ifdef USE_POPPLER
  /// Poppler PDF engine
  PopplerEngine = 1,
#endif
#ifdef USE_MUPDF
  /// MuPDF PDF engine
  MuPdfEngine = 2,
#endif
};

/// @todo restructure namespaces
namespace renderer
{
/// Type of PDF renderer.
enum Renderer {
#ifdef USE_QTPDF
  QtPDF = 0,
#endif
#ifdef USE_POPPLER
  Poppler = 1,
#endif
#ifdef USE_MUPDF
  MuPDF = 2,
#endif
#ifdef USE_EXTERNAL_RENDERER
  ExternalRenderer = 3,
#endif
};
}  // namespace renderer

/// Mode for handling drawings in overlays.
/// Overlays are PDF pages sharing the same label.
enum OverlayDrawingMode {
  /// Unknown mode, used to indicate invalid user input
  InvalidOverlayMode,
  /// Every page has independent drawings.
  PerPage,
  /// All pages with the same label in a simply connected region
  /// have the same drawings.
  PerLabel,
  /// When going to the next page which has the same label, the
  /// current drawings are copied.
  Cumulative,
};

/**
 * @brief Use single bits of an integer to mark skipping of overlays.
 *
 * Page shifts are stored as integers in SlideScenes.
 * The information about whether overlays should be considered is
 * stored in the bits controlled by FirstOverlay and LastOverlay.
 * If shift is an int and overlay is of type ShiftOverlays:
 * * shift_overlay = (shift & ~AnyOverlay) | overlay
 * * overlay = shift & AnyOverlay
 * * shift = shift >= 0 ? shift & ~AnyOverlay : shift | AnyOverlay
 */
enum ShiftOverlays {
  /// No overlay, only consider page index for shifting.
  NoOverlay = 0,
  /// Only consider first overlay of each slide when shifting.
  FirstOverlay = (INT_MAX >> 2) + 1,
  /// Only consider last overlay of each slide when shifting.
  LastOverlay = (INT_MAX >> 1) + 1,
  /// Any overlay is used.
  AnyOverlay = (LastOverlay | FirstOverlay),
};

/// Actions triggered by keyboard shortcuts, buttons, or some widgets.
enum Action {
  InvalidAction = 0,  ///< invalid actoin
  NoAction,           ///< no action
  // Nagivation actions
  Update,                ///< refresh view by navigating to current page
  NextPage,              ///< navigate to next page
  PreviousPage,          ///< navigate to previous page
  NextSkippingOverlays,  ///< navigate to next page which has a different page
                         ///< label than the current page
  PreviousSkippingOverlays,  ///< navigate to previous page which has a
                             ///< different page label than the current page
  FirstPage,                 ///< navigate to first page in the document
  LastPage,                  ///< navigate to last page in the document
  // Drawing
  UndoDrawing,   ///< undo last drawing step on current page (only FullPage)
  RedoDrawing,   ///< redo undone drawing step on current page (only FullPage)
  ClearDrawing,  ///< clear all drawings on current page (only FullPage)
  ScrollDown,    ///< scroll slide down
  ScrollUp,      ///< scroll slide up
  ScrollNormal,  ///< scroll slide back to normal
  SaveDrawings,  ///< save drawings to known file name, ask for file name if no
                 ///< file name is known
  SaveDrawingsAs,       ///< save drawings to file, always ask for file name
  LoadDrawings,         ///< load drawings from file, always ask for file name
  LoadDrawingsNoClear,  ///< load drawings from file without clearing existing
                        ///< drawings (better don't use that!)
  // Modify drawn items
  CopyClipboard,          ///< copy selection to clipboard
  CutClipboard,           ///< cut selection to clipboard
  PasteClipboard,         ///< paste from clipboard
  SelectionToForeground,  ///< bring selected items to foreground
  SelectionToBackground,  ///< bring selected items to background
  RemoveSelectedItems,    ///< remove all selected items
  SelectAll,              ///< select all items on currently focused slide
  ClearSelection,         ///< clear selection on currently focused slide
  // Timer
  StartTimer,       ///< start timer
  StopTimer,        ///< stop (pause) timer
  StartStopTimer,   ///< toggle timer state (running/paused)
  ResetTimePassed,  ///< set passed time to zero
  // Media
  PlayMedia,       ///< play all media objects on current page
  PauseMedia,      ///< pause all media objects on current page
  PlayPauseMedia,  ///< pause all media objects on current page if a media
                   ///< object is running; otherwise play all media objects on
                   ///< current page
  Mute,            ///< mute all media objects
  Unmute,          ///< unmute all media objects
  // Other actions
  ReloadFiles,  ///< reload PDF file(s) if it has changed
  FullScreen,   ///< toggle full screen mode
  Quit,         ///< quit program, ask to save if there are unsaved changes
  QuitNoConfirmation,  ///< quit program ignoring about unsaved changes
  // Internal actions
  PdfFilesChanged,  ///< notify widgets that PDF files have changed
  ResizeViews,      ///< slide views have been resized

  // Actions containing flags
  UndoDrawingLeft =
      UndoDrawing | PagePart::LeftHalf,  ///< undo last drawing step on current
                                         ///< page (only LeftPage)
  UndoDrawingRight =
      UndoDrawing | PagePart::RightHalf,  ///< undo last drawing step on current
                                          ///< page (only RightPage)
  RedoDrawingLeft =
      RedoDrawing | PagePart::LeftHalf,  ///< redo undone drawing step on
                                         ///< current page (only LeftPage)
  RedoDrawingRight =
      RedoDrawing | PagePart::RightHalf,  ///< redo undone drawing step on
                                          ///< current page (only RightPage)
  ClearDrawingLeft =
      ClearDrawing | PagePart::LeftHalf,  ///< clear all drawings on current
                                          ///< page (only LeftPage)
  ClearDrawingRight =
      ClearDrawing | PagePart::RightHalf,  ///< clear all drawings on current
                                           ///< page (only RightPage)
};

Q_DECLARE_METATYPE(Action);

/**
 * @brief Gestures to which actions can be assigned.
 */
enum Gesture {
  InvalidGesture,  ///< invalid gesture
  SwipeLeft,       ///< swipe left (on touch screen)
  SwipeUp,         ///< swipe up (on touch screen)
  SwipeDown,       ///< swipe down (on touch screen)
  SwipeRight,      ///< swipe right (on touch screen)
};

/**
 * @brief Widget types used in modular GUI.
 */
enum GuiWidget {
  InvalidType = 0,    ///< Plain QWidget
  VBoxWidgetType,     ///< ContainerWidget with vertical FlexLayout
  HBoxWidgetType,     ///< ContainerWidget with horizontal FlexLayout
  StackedWidgetType,  ///< StackedWidget
  TabbedWidgetType,   ///< TabWidget
  SlideType,          ///< SlideView
  OverviewType,       ///< ThumbnailWidget
  TOCType,            ///< TOCwidget
  NotesType,          ///< NotesWidget
  ToolSelectorType,   ///< ToolSelectorWidget
  ToolWidgetType,     ///< ToolWidget
  SettingsType,       ///< SettingsWidget
  ClockType,          ///< ClockWidget
  AnalogClockType,    ///< AnalogClockWidget
  SearchType,         ///< SearchWidget
  TimerType,          ///< TimerWidget
  SlideNumberType,    ///< SlideNumberWidget
  SlideLabelType,     ///< SlideLabelWidget
};

/// Integer values added to QGraphicsItem::UserType to define custom
/// QGraphicsItem types
enum CustomGraphicsItemTypes {
  BasicGraphicsPathType = 1,
  FullGraphicsPathType = 2,
  FlexGraphicsLineItemType = 3,
  PixmapGraphicsItemType = 4,
  TextGraphicsItemType = 5,
  RectGraphicsItemType = 6,
  EllipsisGraphicsItemType = 7,
  LineGraphicsItemType = 8,
  ArrowGraphicsItemType = 9,
  SelectionRectItemType = 10,
  GraphicsPictureItemType = 11,
  AudioItemType = 12,
  VideoItemType = 13,
};

#ifdef QT_DEBUG
/// Debug flags: combinable debug flags
enum DebugFlag {
  NoLog = 0,
  DebugRendering = 1 << 0,
  DebugCache = 1 << 1,
  DebugDrawing = 1 << 2,
  DebugMedia = 1 << 3,
  DebugKeyInput = 1 << 4,
  DebugOtherInput = 1 << 5,
  DebugSettings = 1 << 6,
  DebugTransitions = 1 << 7,
  DebugPageChange = 1 << 8,
  DebugLayout = 1 << 9,
  DebugWidgets = 1 << 10,
  DebugThreads = 1 << 11,
  DebugFunctionCalls = 1 << 12,
  DebugAll = 0x7fff,
  DebugVerbose = 1 << 15,
};
#endif  // QT_DEBUG

#endif  // ENUMERATES_H
