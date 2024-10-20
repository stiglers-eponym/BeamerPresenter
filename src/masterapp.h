// SPDX-FileCopyrightText: 2024 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MASTERAPP_H
#define MASTERAPP_H

#include <QApplication>
#include <QFileInfo>
#include <QIcon>
#include <QTabletEvent>
#include <memory>

#include "src/config.h"
#include "src/drawing/pointingtool.h"
#include "src/drawing/tool.h"
#include "src/preferences.h"

#ifdef USE_TRANSLATIONS
#include <QTranslator>
#endif

#ifdef USE_POPPLER
#if (QT_VERSION_MAJOR == 6)
#if __has_include(<poppler/qt6/poppler-version.h>)
#include <poppler/qt6/poppler-version.h>
#else
#define POPPLER_VERSION "OLD"
#endif
#elif (QT_VERSION_MAJOR == 5)
#if __has_include(<poppler/qt5/poppler-version.h>)
#include <poppler/qt5/poppler-version.h>
#else
#define POPPLER_VERSION "OLD"
#endif
#endif  // QT_VERSION_MAJOR
#endif  // USE_POPPLER
#ifdef USE_MUPDF
extern "C" {
#include <mupdf/fitz/version.h>
}
#endif

/**
 * @brief MasterApp class, minimal extension of QApplication
 *
 * This class reimplements event handling to handle table proximity
 * leave events.
 */
class MasterApp : public QApplication
{
#ifdef USE_TRANSLATIONS
  QTranslator translator;
#endif

 public:
  MasterApp(int &argc, char **argv) : QApplication(argc, argv) {}

  ~MasterApp() {}

  void initialize(const QString &fallback_root)
  {
    setApplicationName("BeamerPresenter");
    // Set app version. The string APP_VERSION is defined in src/config.h.
    setApplicationVersion(APP_VERSION
#ifdef USE_POPPLER
                          " poppler=" POPPLER_VERSION
#endif
#ifdef USE_MUPDF
                          " mupdf=" FZ_VERSION
#endif
                          " Qt=" QT_VERSION_STR
#ifdef QT_DEBUG
                          " debugging"
#endif
    );
    // Load the icon
    setWindowIcon(QIcon(QFileInfo::exists(ICON_FILEPATH)
                            ? ICON_FILEPATH
                            : fallback_root + ICON_FILEPATH));

#ifdef USE_TRANSLATIONS
    QString translation_path = TRANSLATION_PATH;
    if (!QFileInfo::exists(translation_path)) {
      translation_path = fallback_root + translation_path;
      if (!QFileInfo::exists(translation_path))
        translation_path = fallback_root + "/locale";
    }
    for (auto &lang : QLocale().uiLanguages())
      if (translator.load(
              "beamerpresenter.qm",
              translation_path + lang.replace('-', '_') + "/LC_MESSAGES")) {
        installTranslator(&translator);
        break;
      }
#endif
  }

 protected:
  /// Handle table proximity leave events: clear pointing tools
  bool event(QEvent *event) override
  {
    switch (event->type()) {
      case QEvent::TabletLeaveProximity: {
        const auto tevent = dynamic_cast<const QTabletEvent *>(event);
        if (tevent == nullptr) return false;
        const int device = tablet_event_to_input_device(tevent);
        for (const auto &tool : std::as_const(preferences()->current_tools)) {
          if (tool && (tool->device() & device) &&
              (tool->tool() & Tool::AnyPointingTool)) {
            auto ptool = std::dynamic_pointer_cast<PointingTool>(tool);
            if (ptool) ptool->clearPos();
          }
        }
        return true;
      }
      default:
        return QApplication::event(event);
    }
  }
};

#endif  // MASTERAPP_H
