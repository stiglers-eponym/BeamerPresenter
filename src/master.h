// SPDX-FileCopyrightText: 2022 Valentin Bruch <software@vbruch.eu>
// SPDX-License-Identifier: GPL-3.0-or-later OR AGPL-3.0-or-later

#ifndef MASTER_H
#define MASTER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QRectF>
#include <QKeySequence>
#include "src/config.h"
#include "src/preferences.h"
#include "src/enumerates.h"

class Tool;
class QColor;
class QTime;
class QTimerEvent;
class QString;
class QJsonObject;
class QWidget;
class QKeyEvent;
class PdfMaster;
class SlideScene;
class PixCache;
class QMainWindow;
class QXmlStreamReader;
class QXmlStreamWriter;

/**
 * @brief Central management of the program.
 *
 * Manage the program, distributes events to various objects, construct the
 * GUI. All changes to preferences must be done though this class (this will
 * change when a GUI for modifying preferences is implemented).
 * Only one master object may exist.
 *
 * Only a single Master object should be used. A pointer to this master object
 * is accessible with the global master() function.
 */
class Master : public QObject
{
    Q_OBJECT

    /// List of all PDF documents.
    /// Master file is the first entry in this list.
    QList<PdfMaster*> documents;

    /// Map of cache hashs to cache objects.
    QMap<int, const PixCache*> caches;

    /// List of all windows of the applications.
    QList<QMainWindow*> windows;

    /// Key sequence shortcuts for focusing widgets.
    QMap<QKeySequence, QWidget*> shortcuts;

    /// Timer to tell slides that they should start caching videos for the next slide.
    int cacheVideoTimer_id {-1};

    /// Timer for automatic slide changes.
    int slideDurationTimer_id {-1};

    /// Ask for confirmation when closing.
    /// Return true when the program should quit.
    bool askCloseConfirmation() const noexcept;

public:
    /// Constructor: initializes times.
    Master();

    /// Destructor: delete cache, scenes, documents, and windows.
    /// This should delete everything.
    ~Master();

    /// Show all windows of the application.
    void showAll() const;

    enum Status {
        Success = 0, ///< at least one window was created and at least one document was loaded.
        ReadConfigFailed = 1, ///< reading the config file failed.
        ParseConfigFailed = 2, ///< parsing the config file failed.
        NoWindowsCreated = 3, ///< no windows were created for any other reason.
        NoPDFLoaded = 4, ///< no PDF file was loaded.
    };

    /// Read configuration file and build up GUI.
    Status readGuiConfig(const QString& filename);

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

    /// Get save file name from QFileDialog
    static QString getSaveFileName();
    /// Get open file name from QFileDialog
    static QString getOpenFileName();

protected:
    /// Timeout event: cache videos or change slide
    void timerEvent(QTimerEvent *event) override;

    /// Filter key input events from other widgets
    bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
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
    void navigateToPage(const int page);

    /// Navigate to next slide.
    void nextSlide() noexcept
    {navigateToPage(preferences()->page + 1);}

    /// Handle an action, distribute it if necessary.
    void handleAction(const Action action);

    /// Set currently used tool. This takes ownership of tool.
    void setTool(Tool *tool) const noexcept;

    /// finish navigation event: called after page change or after slide transition.
    void postNavigation() noexcept;

    /// Show an error message as QMessageBox::critical
    void showErrorMessage(const QString &title, const QString &text) const;

signals:
    /// Send out new tool to SlideScenes (changes selected items).
    void sendNewToolScene(const Tool *tool) const;
    /// Send out new tool only to tool buttons to update icons.
    /// This signal must always be sent when a tool changes,
    /// which is currently connected to a device.
    void sendNewToolSoft(const Tool *tool) const;
    /// Send out new stroke color.
    void sendColor(const QColor &color) const;
    /// Send out new stroke width.
    void sendWidth(const qreal width) const;
    /// Send out action.
    void sendAction(const Action action) const;
    /// Set status for an action (e.g. timer paused or running).
    void sendActionStatus(const Action action, const int status) const;
    /// Set memory of each PixCache object to scale*(number of pixels)
    void sendScaledMemory(const float scale);
    /// Clear cache of all PixCache objects
    void clearCache();
    /// Tell slide scenes to start post-rendering operations.
    void postRendering();

    /// Prepare navigation: Scenes update geometry, such that the layout can
    /// be recalculated.
    void prepareNavigationSignal(const int page) const;

    /// Send out navigation signal (after updating preferences()->page).
    /// This should only be used in queued connection.
    void navigationSignal(const int page) const;

    /// Set end time (in ms) for page.
    void setTimeForPage(const int page, const quint32 time);
    /// Get end time (in ms) for page. time is set to UINT32_MAX if no end time is defined.
    void getTimeForPage(const int page, quint32 &time) const;
    /// Tell NotesWidget to write notes to writer.
    void writeNotes(QXmlStreamWriter &writer);
    /// Tell NotesWidget to read notes from reader.
    void readNotes(QXmlStreamReader &reader);
    /// Notify TimerWidget of changes in total time.
    void setTotalTime(const QTime time) const;
    /// Tell PdfMaster to save drawings.
    void saveDrawings(const QString filename);
    /// Tell PdfMaster to load drawings. This does not clear old drawings
    /// before loading new ones.
    void loadDrawings(const QString filename);
};

#endif // MASTER_H
