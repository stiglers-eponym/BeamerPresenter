// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef TOCWIDGET_H
#define TOCWIDGET_H

#include <QScrollArea>
#include <QSize>
#include <memory>

#include "src/config.h"
#include "src/gui/tocbutton.h"

class PdfDocument;
class PdfOutlineEntry;
class QKeyEvent;
class QShowEvent;
class QFocusEvent;
class QGridLayout;

/**
 * @brief Widget showing document outline.
 *
 * The document outline is saved as a tree structure of TOCbuttons.
 * The tree root is first_button.
 *
 * @see TOCbutton
 */
class TOCwidget : public QScrollArea
{
  Q_OBJECT

  /// Root of TOCbutton tree representing the outline.
  TOCbutton *first_button{nullptr};

  /// Document, for which table of content is shown.
  std::shared_ptr<const PdfDocument> document;

  /// Icon for expanding the tree view.
  QIcon expand_icon;

  /// Number of items in TOC.
  int num_items = 0;

  TOCbutton *addButtons(const QVector<PdfOutlineEntry> &output,
                        QGridLayout *layout, int idx, const int depth);

  void expandToButton(TOCbutton *button, const int page);

 public:
  /// Trivial constructor, does not create the outline tree.
  explicit TOCwidget(std::shared_ptr<const PdfDocument> document,
                     QWidget *parent = nullptr)
      : QScrollArea(parent), document(document)
  {
  }

  /// Destructor: TOCbuttons are deleted recursively.
  ~TOCwidget() { delete first_button; }

  /// Generate the TOC from given document or preferences()->document.
  void generateTOC();

  /// Actually this is nonsense, but currently the layout only works with
  /// this option set.
  bool hasHeightForWidth() const override { return true; }

  /// Size hint required by layout.
  QSize sizeHint() const noexcept override { return {100, 200}; }

 public slots:
  /// Show event: generate outline if necessary. Expand to current position.
  void showEvent(QShowEvent *) override;

  /// Override key press events: Send page up and page down to master.
  void keyPressEvent(QKeyEvent *event) override;

  /// Focus event: generate outline if necessary.
  void focusInEvent(QFocusEvent *) override { generateTOC(); }

  /// Expand all sections, subsections, ... which contain the given page.
  void expandToPage(const int page);
};

#endif  // TOCWIDGET_H
