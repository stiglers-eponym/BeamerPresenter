#ifndef MASTER_H
#define MASTER_H

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMediaPlaylist>
#include <QSizePolicy>
#include "src/enumerates.h"
#include "src/preferences.h"

class Tool;
class PdfMaster;
class SlideScene;
class PixCache;
class QMainWindow;

/**
 * Manage the program, distributes events to various objects, construct the
 * GUI. All changes to preferences must be done though this class (this will
 * change when a GUI for modifying preferences is implemented).
 * Only one master object may exist.
 */
class Master : public QObject
{
    Q_OBJECT

    /// List of all PDF documents.
    /// Master file is the first entry in this list.
    QList<PdfMaster*> documents;

    /// Map of cache hashs to cache objects.
    QMap<int, PixCache*> caches;

    /// List of all windows of the applications.
    QList<QMainWindow*> windows;

    /// Playlist of all media content in the documents.
    QMediaPlaylist *playlist {NULL};

    /// Map file names (urls) to playlist indices.
    QMap<QUrl, int> playlist_map;

    /// Key sequence shortcuts for focusing widgets.
    QMap<quint32, QWidget*> shortcuts;

    /// Timer to tell slides that they should start caching videos for the next slide.
    QTimer *cacheVideoTimer {NULL};

    /// Timer for automatic slide changes.
    QTimer *slideDurationTimer {NULL};

public:
    /// Constructor: initializes times.
    Master();

    /// Destructor: delete cache, scenes, documents, and windows.
    /// This should delete everything.
    ~Master();

    /// Show all windows of the application.
    void showAll() const;

    /**
     *  Read configuration file and build up GUI. Return values are:
     *  0 if at least one window was created and at least one document was loaded.
     *  1 if reading the config file failed.
     *  2 if parsing the config file failed.
     *  3 if no windows were created for any other reason.
     *  4 if no PDF file was loaded.
     */
    char readGuiConfig(const QString& filename);

    /// Create widgets recursively.
    QWidget* createWidget(QJsonObject& object, QWidget *parent = NULL);

    /// Calculate total cache size (sum up cache sizes from all PixCache objects).
    qint64 getTotalCache() const;

    /**
     * A navigation event moves preferences()->page away from the given page.
     * Tell path containers in all documents that history of given page
     * should now be limited by preferences()->history_length_hidden_slides.
     * page is given as (page_number | page_part).
     * Currently this also notifies the layout system of changes if
     * PDFs with flexible page sizes are used.
     */
    void leavePage(const int page) const;

public slots:
    /// Handle key events collected by any other object.
    /// By default all key events are redirected to this and handled here.
    void receiveKeyEvent(const QKeyEvent* event);

    /// Read memory size restriction from preferences and distribute memory to pixcaches.
    void distributeMemory();

    /**
     * @brief Distribute navigation events
     *
     * 1. check whether page is valid.
     * 2. truncate drawing history on current page (if necessary)
     * 3. update scene geometries based on page size
     * 4. recalculate geometry
     * 5. update page in preferences
     * 6. send out navigation signal
     */
    void navigateToPage(const int page) const;

    /// Navigate to next slide.
    void nextSlide() const noexcept
    {navigateToPage(preferences()->page + 1);}

    /// Handle an action, distribute it if necessary.
    void handleAction(const Action action);

    /// This takes ownership of tool.
    void setTool(Tool *tool) const noexcept;

    /// finish navigation event: called after page change or after slide transition.
    void postNavigation() const noexcept;

signals:
    /// Send out action.
    void sendAction(const Action action) const;

    /// Prepare navigation: Scenes update geometry, such that the layout can
    /// be recalculated.
    void prepareNavigationSignal(const int page) const;

    /// Send out navigation signal (after updating preferences()->page).
    /// This should only be used in queued connection.
    void navigationSignal(const int page) const;
};

#endif // MASTER_H
