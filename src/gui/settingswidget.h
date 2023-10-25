// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QSize>
#include <QTabWidget>

#include "src/config.h"

class QTextEdit;

/**
 * @brief Graphical interface to writable_preferences()
 *
 * This must always run in the same thread as preferences(). It contains
 * 4 tabs: a manual, general settings, shortcuts and rendering settings.
 */
class SettingsWidget : public QTabWidget
{
  Q_OBJECT

  /// Manual
  QTextEdit *manual;
  /// General settings
  QWidget *misc;
  /// List and modify keyboard shortcuts
  QWidget *shortcuts;
  /// Settings affecting rendering and cache
  QWidget *rendering;

  /// Initialize manual tab.
  void initManual();
  /// Initialize misc tab.
  void initMisc();
  /// Initialize shortcuts tab.
  void initShortcuts();
  /// Initialize rendering tab.
  void initRendering();

 public:
  /// Construct all 4 tabs.
  explicit SettingsWidget(QWidget *parent = nullptr);

  /// Size hint for layout.
  QSize sizeHint() const noexcept override { return {100, 200}; }

  /// required by layout.
  bool hasHeightForWidth() const noexcept override { return true; }

 private slots:
  /// Create a new keyboard shortcut.
  void appendShortcut();

  /// Select GUI config file from QFileDialog
  void setGuiConfigFile();
};

#endif  // SETTINGSWIDGET_H
