#ifndef MASTER_H
#define MASTER_H

#include <QTabWidget>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMediaPlaylist>
#include <QSizePolicy>
#include "src/enumerates.h"
#include "src/names.h"
#include "src/gui/flexlayout.h"
#include "src/gui/clockwidget.h"
#include "src/gui/stackedwidget.h"
#include "src/gui/containerwidget.h"
#include "src/gui/slidenumberwidget.h"

class PdfMaster;
class SlideScene;
class PixCache;

/// Manage the program, distributes events to various objects, construct the
/// GUI. All changes to preferences must be done though this class (this will
/// change when a GUI for modifying preferences is implemented).
/// Only one master object may exist.
class Master : public QObject
{
    Q_OBJECT

    /// List of all PDF documents.
    /// Master file is the first entry in this list.
    QList<PdfMaster*> documents;

    /// Map of cache hashs to cache objects.
    QMap<int, PixCache*> caches;

    /// List of all windows of the applications.
    QList<QWidget*> windows;

    /// Playlist of all media content in the documents.
    QMediaPlaylist *playlist {nullptr};

    /// Map file names (urls) to playlist indices.
    QMap<QUrl, int> playlist_map;

    /// Key sequence shortcuts for focusing widgets.
    QMap<quint32, QWidget*> shortcuts;

public:
    /// Trivial constructor. Doesn't do anything.
    Master() {}

    /// Destructor: delete cache, scenes, documents, and windows.
    /// This should delete everything.
    ~Master();

    /// Show all windows of the application.
    void showAll() const;

    /// Read configuration file and build up GUI. Return true on success.
    bool readGuiConfig(const QString& filename);

    /// Create widgets recursively.
    QWidget* createWidget(QJsonObject& object, QWidget *parent = nullptr);

    /// Calculate total cache size (sum up cache sizes from all PixCache objects).
    qint64 getTotalCache() const;

    /// A nagivation event moves preferences().page away from the given page.
    /// Tell path containers in all documents that history of given page
    /// should now be limited by preferences().history_length_hidden_slides.
    /// page is given as (page_number | page_part).
    /// TODO: currently this also notifies the layout system of changes if
    /// PDFs with flexible page sizes are used.
    void limitHistoryInvisible(const int page) const;

public slots:
    /// Handle key events collected by any other object.
    /// By default all key events are redirected to this and handled here.
    void receiveKeyEvent(QKeyEvent const* event);

    /// Read memory size restriction from preferences and distribute memory to pixcaches.
    void distributeMemory();

    /// Distribute navigation events.
    void navigateToPage(const int page);

signals:
    /// Send out action.
    void sendAction(const Action action) const;

    /// Send out nagivation signal (after updating preferences().page).
    void navigationSignal(const int page) const;
};

#endif // MASTER_H
