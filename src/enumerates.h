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
  /// Unknown page part
  UnknownPagePart = 1,
};

/**
 * @brief PPage: page and page part identifies what is shown in one slide.
 *
 * A "slide" as shown in a slide widget can be either a full page of a PDF
 * document, or just half of a page. LaTeX beamer has an option to create
 * PDF pages into the presentation (left half) and notes for the speaker
 * (right half). Thus, the correct identifier for what is shown in a slide
 * widget is PPage (page index + page part).
 */
struct PPage {
  int page;
  PagePart part;
};

inline bool operator<(PPage a, PPage b) noexcept
{
  return a.page < b.page || (a.page == b.page && a.part < b.part);
}
inline bool operator>(PPage a, PPage b) noexcept
{
  return a.page > b.page || (a.page == b.page && a.part > b.part);
}
inline bool operator==(PPage a, PPage b) noexcept
{
  return a.page == b.page && a.part == b.part;
}

Q_DECLARE_METATYPE(PPage);

/// PDF engine
enum class PdfEngine {
#ifdef USE_QTPDF
  /// Internal Qt PDF engine (Qt PDF)
  QtPDF = 0,
#endif
#ifdef USE_POPPLER
  /// Poppler PDF engine
  Poppler = 1,
#endif
#ifdef USE_MUPDF
  /// MuPDF PDF engine
  MuPdf = 2,
#endif
};

Q_DECLARE_METATYPE(PdfEngine);

/// Type of PDF renderer.
enum class Renderer {
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

Q_DECLARE_METATYPE(Renderer);

/// Mode for handling drawings in overlays.
/// Overlays are PDF pages sharing the same label.
enum class OverlayDrawingMode {
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

Q_DECLARE_METATYPE(OverlayDrawingMode);

/// How to interpret a page shift when a presentation contains overlays.
enum class ShiftOverlays {
  /// Ignore overlays, only consider slide index when shifting.
  NoOverlay = 0,
  /// Only consider first overlay of each slide when shifting.
  FirstOverlay,
  /// Only consider last overlay of each slide when shifting.
  LastOverlay,
};

Q_DECLARE_METATYPE(ShiftOverlays);

/**
 * @brief PageShift: integer shift + information how to handle overlays
 *
 * A slide widget can show a preview of the next slide. More generally, you can
 * show the slide obtained by a general shift relative to the current slide.
 * PageShift encodes this shift. It consists of an integer shift and the
 * overlay information that determines how to interpret the shift.
 * For example, this can be the last overlay of the shifted page.
 */
struct PageShift {
  int shift;
  ShiftOverlays overlay;
};

inline bool operator<(PageShift a, PageShift b) noexcept
{
  return a.shift < b.shift || (a.shift == b.shift && a.overlay < b.overlay);
}
inline bool operator>(PageShift a, PageShift b) noexcept
{
  return a.shift > b.shift || (a.shift == b.shift && a.overlay > b.overlay);
}
inline bool operator==(PageShift a, PageShift b) noexcept
{
  return a.shift == b.shift && a.overlay == b.overlay;
}

Q_DECLARE_METATYPE(PageShift);

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
  ZoomIn,        ///< zoom in on a slide
  ZoomOut,       ///< zoom out of a slide
  ZoomReset,     ///< reset slide zoom
  SaveDrawings,  ///< save drawings to known file name, ask for file name if no
                 ///< file name is known
  SaveDrawingsAs,       ///< save drawings to file, always ask for file name
  LoadDrawings,         ///< load drawings from file, always ask for file name
  LoadDrawingsNoClear,  ///< load drawings from file without clearing existing
                        ///< drawings (better don't use that!)
  ExportDrawingsSvg,    ///< Export drawings to SVG images.
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
  InsertSlide,   ///< insert an empty slide
  RemoveSlide,   ///< remove the current slide
  RestoreSlide,  ///< restore a previously removed slide
  ReloadFiles,   ///< reload PDF file(s) if it has changed
  FullScreen,    ///< toggle full screen mode
  Quit,          ///< quit program, ask to save if there are unsaved changes
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
